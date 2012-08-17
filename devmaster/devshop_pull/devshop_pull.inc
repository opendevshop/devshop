<?php


/**
 * Menu callback that is invoked by GitHub WebHook facility to create
 * a code pull task.
 *
 * @TODO: Save "Last Pull" and "Last Pull Status" when git pull task is complete.
 */
function devshop_pull_callback($platform, $hash) {
  
  // Get the access from list
  $acl = explode("\n", trim(variable_get('devshop_pull_ip_acl', '')));

  // I wish I didn't have to do this
  for($i = 0; $i < count($acl); $i++) {
    $acl[$i] = trim($acl[$i]);
  }

  // Make sure the client's IP address is on the list
  if (!in_array(ip_address(), $acl)) {
    $message = ip_address() . " is not authorized to invoke a Pull Code request.";
    watchdog(DSGH_WD_TYPE, $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }

  $message = "GitHub post-receive hook invoked by " . ip_address();
  watchdog(DSGH_WD_TYPE, $message, array(), WATCHDOG_INFO);
  print "$message<br>";


  if (strlen($platform) < 1 || strlen($hash) != 32) {
    $message = "Invalid/missing parameters in URL!";
    watchdog(DSGH_WD_TYPE, $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }

  // Based on the platform name, get the node ID for the platform node
  $pnid = db_result(db_query(
    "SELECT nid FROM {hosting_context} WHERE name = '%s'", $platform));

  // Load the entire node
  if (!$pnode = node_load($pnid)) {
    $message = "Unable to load platform node!";
    watchdog(DSGH_WD_TYPE, $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }

  // Make sure the security code is valid
  if (_devshop_pull_hash_create($pnode) != $hash) {
    $message = "Security code $hash is not valid!";
    watchdog(DSGH_WD_TYPE, $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }
  
  // Make sure the platform has pull callback enabled
  if ($pnode->pull_method != DEVSHOP_PULL_CALLBACK){
    $message = "Platform is NOT configured to use Pull Code URL callback!";
    watchdog(DSGH_WD_TYPE, $message, array(), WATCHDOG_ERROR);
    print "$message<br>";
    return;
  }

  // Iterate through all of the sites that use this platform
  $r = db_query("SELECT nid FROM {hosting_site} " .
                "WHERE platform = %d AND status = 1", $pnid);

  // When doing the pull task, don't do a db update or revert. 
  // Just clear all of the caches
  $args = array(
    'update' => 0,
    'revert' => 0,
    'cache'  => 1
  );

  while ($site = db_fetch_object($r)) {
    $message = "Queuing a pull task for node id $site->nid";
    watchdog(DSGH_WD_TYPE, $message, array(), WATCHDOG_INFO);
    print "$message<br>";
    hosting_add_task($site->nid, 'devshop-pull', $args);
  } 
}

/**
 * Create the full URL that is displayed in the platform node view
 * and given to the GitHub WebHook to invoke a pull after a commit.
 */
function _devshop_pull_callback_url($node) {
  return url(DEVSHOP_PULL_CALLBACK_URL .
	'/' . $node->hosting_name .
	'/' . _devshop_pull_hash_create($node), 
	array('absolute' => TRUE));
}

/**
 * Create a security hash code based on the platform node
 */
function _devshop_pull_hash_create($node) {
  return md5($node->hosting_name . $node->nid);
}
