<?php


/**
 * Menu callback that is invoked by GitHub WebHook facility to create
 * a code pull task.
 *
 * @TODO: Save "Last Pull" and "Last Pull Status" when git pull task is complete. ... DONE?? seems to be in place
 */
function devshop_pull_callback($project, $hash) {
  
  // @TODO: Should we switch to using node ID so we can use hook_menu's autoloader?
  
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
  elseif ($project_node->pull_method != DEVSHOP_PULL_CALLBACK){
    $message = "Project $project is NOT configured to use Pull Code URL callback!";
  }
  // Make sure the project has platforms with pull enabled.
  elseif (empty($environments_to_pull)){
    $message = "Project $project has no environments configured to Pull Code on commit!";
  }
  // Make sure the client's IP address is on the list
  else if (!in_array(ip_address(), $allowed_ips)) {
    $message = ip_address() . " is not authorized to invoke a Pull Code request.";
    $status = DEVSHOP_PULL_STATUS_ACCESS_DENIED;
  }
  // All checks pass! Server is allowed to trigger tasks!
  else {
    $message = "Commit Received! Invoked by " . ip_address();
    $status = DEVSHOP_PULL_STATUS_OK;
    
    // @TODO: Check Payload Here... If we see a GitHub payload with branches,
    // filter out the $environments_to_pull based on those branches.
    
    // Create the hosting task
    // We need to pass environments so we can control what envs to pull based
    // on the data coming back from github!
    $args = array();
    $args['environments'] = implode(' ', $environments_to_pull);
    hosting_add_task($project_node->nid, 'devshop-pull', $args);
  }

  // Log It, only if there is a status
  if (isset($status)){
    $record = new stdClass;
    $record->project_nid = $project_node->nid;
    $record->pull_method = $project_node->pull_method;
    $record->last_pull = time();
    $record->last_pull_status = $status;
    $record->last_pull_ip = ip_address();
    
    drupal_write_record('hosting_devshop_pull_projects', $record, array('project_nid'));
  }

  // Output a message, no matter what.  
  watchdog('devshop_pull', $message, array(), WATCHDOG_INFO);
  print $message;
  
  // Save a variable to help when using the settings page.
  variable_set('devshop_pull_last_ip', ip_address());
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
