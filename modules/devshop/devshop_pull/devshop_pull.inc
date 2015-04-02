<?php

/**
 * URL callback that is invoked by git to create a code pull task.
 */
function devshop_pull_callback($project_hosting_context, $hash) {

  // Load the project node & list of allowed IPs
  $project_node = hosting_context_load($project_hosting_context);
  $project = $project_node->project;
  $deploy_settings = $project_node->project->settings->deploy;

  $allowed_ips  = explode("\n", trim(variable_get('devshop_pull_ip_acl', DEVSHOP_PULL_DEFAULT_ALLOWED_IPS)));
  array_filter(array_map('trim', $allowed_ips));

  // Check for environments set to pull
  $environments_to_pull = array();
  foreach ($project->environments as $environment) {
    if (!$environment->settings->pull_disabled) {
      $environments_to_pull[] = $environment->name;
    }
  }

  // Make sure we got the project.
  if (!$project_node){
    $message = "Project $project not found.";
  }
  // Make sure the security code is valid
  else if (_devshop_pull_hash_create($project_node) != $hash) {
    $message = "Security code $hash is not valid!";
    $status = DEVSHOP_PULL_STATUS_INVALID_CODE;
  }
  // Make sure the project has pull callback enabled
  if ($deploy_settings['method'] != 'webhook') {
    $message = "Project not configured to use webhook deployment.";
  }
  // Make sure the client's IP address is on the list
  else if (!devshop_pull_ip_match(ip_address(), $allowed_ips)) {
    $message = ip_address() . " is not authorized to invoke a webhook request.";
    $status = DEVSHOP_PULL_STATUS_ACCESS_DENIED;
  }
  // All checks pass! Server is allowed to trigger tasks!
  else {
    $status = DEVSHOP_PULL_STATUS_OK;

    // @TODO: Make this pluggable.
    // Check headers for GitHub Integration
    $headers = getallheaders();
    if (isset($headers['X-GitHub-Event'])) {
      $message = devshop_pull_github_webhook($project_node, $environments_to_pull);
    }
    else {
      $message = devshop_pull_default_webhook($project_node, $environments_to_pull);
    }
  }

  // Save the project node with last pull info.
  $deploy_settings['last_webhook'] = time();
  $deploy_settings['last_webhook_status'] = $status;
  $deploy_settings['last_webhook_ip'] = ip_address();

  $project_node->project->settings->deploy = $deploy_settings;

  node_save($project_node);

  // Output a message, no matter what.
  watchdog('devshop_pull', $message, array(), WATCHDOG_INFO);

  // @TODO Print an appropriate header.
  print $message;

  // Save a variable to help when using the settings page.
  variable_set('devshop_pull_last_ip', ip_address());
  variable_set('devshop_pull_last_status', $status);
}

/**
 * Default action to take on webhook init.
 */
function devshop_pull_default_webhook($project_node){
  $project = $project_node->project;
  foreach ($project->environments as $environment) {

    // Respect pull disabled setting.
    if ($environment->settings->pull_disabled) {
      continue;
    }
    $environments_to_pull[] = $environment->name;

    $args = array();
    $args['git_ref'] = $environment->git_ref;
    $args['update'] = $environment->settings->deploy['update'];
    $args['revert'] = $environment->settings->deploy['revert'];
    $args['cache'] = $environment->settings->deploy['cache'];

    hosting_add_task($environment->site, 'devshop-deploy', $args);
  }

  return "Commit notification received! Running 'Deploy' on $project->title environments " . implode(', ', $environments_to_pull);
}

/**
 * GitHub action to take on webhook init
 * @TODO: Once it is more pluggable, put this in it's own include, or maybe it's own module.
 */
function devshop_pull_github_webhook($project_node){
  $headers = getallheaders();
  $project = $project_node->project;

  // @TODO: This only works with GitHub now!!! IMMEDIATELY complete issue https://www.drupal.org/node/2322593
  // so we don't leave all other git hosts in the dark :(

  // @TODO: Handle form content from github as well.
  if ($headers['content-type'] == 'application/json'){
    $data = json_decode($GLOBALS['HTTP_RAW_POST_DATA']);
    
    $args = array();
    $args['cache'] = 1;

    switch ($headers['X-GitHub-Event']){
      case 'ping':
        $message = 'Pong!';
        break;
      case 'push':

        // Limit "Deploy" tasks to only run for the branches we have new code for..
        $git_ref = strtr($data->ref, array('refs/tags/' => '', 'refs/heads/' => ''));

        // Check for environments set to pull
        $environments_to_pull = array();
        foreach ($project->environments as $environment_name => $environment) {

          // Only pull if deploy is not disabled or if environment is tracking a tag.
          if ($git_ref == $environment->git_ref && !$environment->settings->pull_disabled && !in_array($environment->git_ref, $project->settings->git['tags'])) {
            $environments_to_pull[] = $environment->name;

            $args = array();
            $args['git_ref'] = $environment->git_ref;
            $args['update'] = $environment->settings->deploy['update'];
            $args['revert'] = $environment->settings->deploy['revert'];
            $args['cache'] = $environment->settings->deploy['cache'];

            hosting_add_task($environment->site, 'devshop-deploy', $args);
          }
        }

        $message = "Push Received for git ref $git_ref. Deploying code to environments: " . implode(', ', $environments_to_pull);
        break;

      case 'pull_request':
        // If pull request environments is enabled...
        if ($project->settings->github['pull_request_environments']){
          $message = 'Pull Request Received.';

          // @TODO: Handle forks?
          $branch = $data->pull_request->head->ref;

          // Determine environment branch.
          // @TODO: Make Configurable, allow branch names to be env name
          //   $environment_name = "pr" . $data->pull_request->number;
          $environment_name = 'branch_' . str_replace('-', '_', $branch);

          // When PR is opened... create new environment.
          if ($data->action == 'opened'){
            $message = "Detected Pull Request creation for $branch \n";

            // If method is "install"...
            if ($project->settings->github['pull_request_environments_method'] == 'install') {
              hosting_create_environment($project, $environment_name, $branch);
            }
            // Otherwise, it is a clone from live.
            else {
              hosting_create_environment($project, $environment_name, $branch, $project->settings->live['live_environment']);
            }
            $message .= "Environment $environment_name created for $project_node->title \n";
          }

          // When PR is cloded, delete environment.
          elseif ($data->action == 'closed'){
            $message .= "Pull Request Closed \n";
            if ($project->settings->github['pull_request_environments_delete']){
              hosting_add_task($project->environments[$environment_name]->site, 'delete');
              $message .= "Site $environment_name (Node: {$project->environments[$environment_name]->site}) scheduled for deletion.";
            }
          }
        }
        break;
    }

  }
  else {
    $message = 'GitHub Request Received, but not in JSON. Please make sure to configure the webhook to use Payload version: application/vnd.github.v3+json';
  }
  return $message;
}

/**
 * Check whether a given ip address matches a list of allowed ip addresses, some of which
 * may be CIDR.
 *
 * @param $ip
 *   The ip addy to test.
 * @param $list
 *   The list to test against.
 */
function devshop_pull_ip_match($ip, $list) {
  foreach ($list as $cidr) {
    if (trim($ip) === trim($cidr)) {
      return TRUE;
    }
    @list($net, $mask) = explode('/', trim($cidr));
    if (isset($mask)) {
      $bitmask = ~((1 << (32 - $mask)) - 1);
      if ((ip2long($net) & $bitmask) == (ip2long($ip) & $bitmask)) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 * Create the full URL that is displayed in the project node view
 * and given to the GitHub WebHook to invoke a pull after a commit.
 */
function _devshop_pull_callback_url($node) {
  return url(DEVSHOP_PULL_CALLBACK_URL
    . '/' . $node->title
    . '/' . _devshop_pull_hash_create($node),
    array('absolute' => TRUE));
}

/**
 * Create a security hash code based on the platform node
 */
function _devshop_pull_hash_create($node) {
  return md5($node->title . $node->nid);
}

/**
 * Prepares a "Pull Code" task for a project.
 *
 * @param $project_nid
 *   A project nid.
 *
 * Platforms in a project must be enabled to have this command run on them.
 */
function devshop_pull_project($project_nid) {

  $project_node = node_load($project_nid);

  $args = array('environments' => '');

  foreach ($project_node->project->environments as $name => $environment){
    if (!$environment->data->pull_disabled){
      $args['environments'] .= $environment->name .' ';
    }
  }
  $args['environments'] = trim($args['environments']);
  if (!empty($args['environments'])){
    hosting_add_task($project_nid, 'devshop-pull', $args);
  } else {
    print "No environments configured to pull! Aborting.";
  }
}

