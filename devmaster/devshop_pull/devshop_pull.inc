<?php


/**
 * URL callback that is invoked by git to create a code pull task.
 */
function devshop_pull_callback($project, $hash) {

  // Load the project node & list of allowed IPs
  $project_node = hosting_context_load(str_replace('project_', '', $project));
  $allowed_ips  = explode("\n", trim(variable_get('devshop_pull_ip_acl', DEVSHOP_PULL_DEFAULT_ALLOWED_IPS)));
  array_filter(array_map('trim', $allowed_ips));

  // Check for environments set to pull
  $environments_to_pull = array();
  foreach ($project_node->settings as $env => $settings) {
    if ($settings['pull_enabled']) {
      $environments_to_pull[] = $env;
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
  elseif (!$project_node->project_settings['pull']['pull_enabled']){
    $message = "Project $project is NOT configured to use Pull Code URL callback!";
  }
  // Make sure the client's IP address is on the list
  else if (!devshop_pull_ip_match(ip_address(), $allowed_ips)) {
    $message = ip_address() . " is not authorized to invoke a Pull Code request.";
    $status = DEVSHOP_PULL_STATUS_ACCESS_DENIED;
  }
  // All checks pass! Server is allowed to trigger tasks!
  else {
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
  $project_node->project_settings['pull']['last_pull'] = time();
  $project_node->project_settings['pull']['last_pull_status'] = DEVSHOP_PULL_STATUS_OK;
  $project_node->project_settings['pull']['last_pull_ip'] = ip_address();
  node_save($project_node);

  // Output a message, no matter what.
  watchdog('devshop_pull', $message, array(), WATCHDOG_INFO);

  // @TODO Print an appropriate header.
  print $message;

  // Save a variable to help when using the settings page.
  variable_set('devshop_pull_last_ip', ip_address());
}

/**
 * Default action to take on webhook init.
 */
function devshop_pull_default_webhook($project, $environments_to_pull){
  // Create the hosting task
  // We need to pass environments so we can control what envs to pull based
  // on the data coming back from github!
  $args = array();
  $args['environments'] = implode(' ', $environments_to_pull);

  // Always clear cache...
  // @TODO: Should we make this a setting? "What to do on auto-pull?"
  $args['cache'] = 1;
  hosting_add_task($project->nid, 'devshop-pull', $args);

  return "Commit notification received! Running 'Pull Code' on $project->title environments " . $args['environments'];
}

/**
 * GitHub action to take on webhook init
 * @TODO: Once it is more pluggable, put this in it's own include, or maybe it's own module.
 */
function devshop_pull_github_webhook($project){
  $headers = getallheaders();

  // @TODO: Handle form content as well.
  if ($headers['content-type'] == 'application/json'){
    $data = json_decode($GLOBALS['HTTP_RAW_POST_DATA']);
    
    $args = array();
    $args['cache'] = 1;

    switch ($headers['X-GitHub-Event']){
      case 'ping':
        $message = 'Pong!';
        break;
      case 'push':
        // Add a task to pull then project's environments
        // Check for environments set to pull
        $environments_to_pull = array();
        foreach ($project->project_settings['environments'] as $env => $settings) {
          // Only pull if pull disabled or is tracking a tag.
          if (!$settings['pull_disabled'] && !in_array($settings['git_branch'], $project->project_settings['git_tags'])) {
            $environments_to_pull[] = $env;
          }
        }
        $args['environments'] = implode(' ', $environments_to_pull);
        hosting_add_task($project->nid, 'devshop-pull', $args);
        $message = 'Push Received.';
        break;
      case 'pull_request':
        $message = 'Pull Request Received.';

        if ($data->action == 'opened'){
          // @TODO: Add to project settings.
          $branch = $data->pull_request->head->ref;

          $message = "Detected Pull Request for $branch \n";
          if ($project->project_settings['pull']['pull_request_environments']){
            $environment_name = "pr" . $data->pull_request->number;

            if (isset($project->environments[$environment_name])){
              $message = "Environment $environment_name already exists.";
            }
            else {
              hosting_create_environment($project, $environment_name, $branch);
              $message .= "Environment $environment_name created for $project->title \n";
            }
          }
          // @TODO: synchronize action
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
    . '/' . $node->hosting_name
    . '/' . _devshop_pull_hash_create($node),
    array('absolute' => TRUE));
}

/**
 * Create a security hash code based on the platform node
 */
function _devshop_pull_hash_create($node) {
  return md5($node->hosting_name . $node->nid);
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
  // Search platforms with pull enabled for this project
  $results = db_query("SELECT environment FROM {hosting_devshop_pull_platforms} p LEFT JOIN {hosting_devshop_project_object} o ON p.platform_nid = o.object_nid WHERE pull_enabled = 1 AND p.project_nid = %d", $project_nid);

  $args = array('environments' => '');

  while ($info = db_fetch_object($results)){
    $args['environments'] .= $info->environment .' ';
  }
  $args['environments'] = trim($args['environments']);
  if (!empty($args['environments'])){
    hosting_add_task($project_nid, 'devshop-pull', $args);
  } else {
    print "No environments configured to pull! Aborting.";
  }
}

