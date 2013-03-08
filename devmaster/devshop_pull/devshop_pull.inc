<?php


/**
 * Menu callback that is invoked by GitHub WebHook facility to create
 * a code pull task.
 *
 * @TODO: Save "Last Pull" and "Last Pull Status" when git pull task is complete. ... DONE?? seems to be in place
 */
function devshop_pull_callback($project, $hash) {
  
  // Get the access from list
  $acl = explode("\n", trim(variable_get('devshop_pull_ip_acl', DEVSHOP_PULL_DEFAULT_ALLOWED_IPS)));

  // I wish I didn't have to do this
  for($i = 0; $i < count($acl); $i++) {
    $acl[$i] = trim($acl[$i]);
  }

  // Make sure the client's IP address is on the list
  if (!in_array(ip_address(), $acl)) {
    $message = ip_address() . " is not authorized to invoke a Pull Code request.";
    watchdog('devshop', $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }

  // Log It
  $message = "[DEVSHOP] Commit Received! invoked by " . ip_address();
  watchdog('devshop', $message, array(), WATCHDOG_INFO);
  print "$message<br>";


  if (strlen($project) < 1 || strlen($hash) != 32) {
    $message = "Invalid/missing parameters in URL!";
    watchdog('devshop', $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }

  // Based on the project name, get the node ID for the project node.
  // Is need remove the prefix for find the nid.
  $project_nid = db_result(db_query(
    "SELECT nid FROM {hosting_context} WHERE name = '%s'", str_replace('project_', '', $project)));

  // Load the entire node
  if (!$project_node = node_load($project_nid)) {
    $message = "Unable to load project node!";
    watchdog('devshop', $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }

  // Make sure the security code is valid
  if (_devshop_pull_hash_create($project_node) != $hash) {
    $message = "Security code $hash is not valid!";
    watchdog('devshop', $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }
  
  // Make sure the platform has pull callback enabled
  if ($project_node->pull_method != DEVSHOP_PULL_CALLBACK){
    $message = "Project is NOT configured to use Pull Code URL callback!";
    watchdog('devshop', $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }
  
  // This does the hard work of figuring out what tasks to create.
  devshop_pull_project($project_node->nid);
  
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
  hosting_add_task($project_nid, 'devshop-pull', $args);
  
  // Put timestamp
  db_query('UPDATE {hosting_devshop_pull_projects} SET last_pull = %d, last_pull_status = %d WHERE project_nid = %d', time(), 1, $project_nid);
}
