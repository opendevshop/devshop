<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_sync($source = NULL, $destination = NULL) {
  drush_log(dt('Provision DevShop Sync started...'));
  
  //Get source name
  if ($source){
    $source_name = substr($source, 1);  //Without @
  } else {
    $source = drush_get_option('source');
    $source_name = substr($source, 1); //Without @
  }
  //Fail if no source
  if (!$source_name){
    return drush_log(dt('Source not found.  You must enter a source alias to sync from'), 'error');
  }
  
  //Get destination name
  if ($destination){
    $destination_name = substr($destination, 1);  //Without @
  } else {
    //If $destination as an argument is not present it is the target name
    $destination = d()->name;
    $destination_name = substr($destination, 1); //Without @
  }
  
  //Fail if no destination
  if (!$destination_name){
    return drush_log(dt('Destination not found.  You must enter a destination alias to sync from'), 'error');
  }
  
  // Pause Hostmaster
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  //Drop the database
  drush_log(dt('DevShop: Dropping tables for destination'));
  provision_backend_invoke($destination, 'sql-drop');
  
  // Run provision-devshop-pull if requested
  if (drush_get_option('pull')) {
    provision_backend_invoke($destination, 'provision-git-pull');
  }
  
  // Sync the databases
  $timestamp = date("Ymd-His");
  $filepath = drush_server_home() . "/backups/$source_name-$timestamp.sql";
  $options = array(
    'result-file' => $filepath,
  );
  //Source and target dump options aren't working here...
  //provision_backend_invoke($destination, 'sql-sync', array($source, $destination), $options);

  
  // SQL Dump
  drush_log(dt('DevShop: SQL Dump for !source', array('!source' => $source)));
  provision_backend_invoke($source, 'sql-dump', array(), $options);

  // SQL Import
  drush_log(dt('DevShop: SQL Import to !destination', array('!destination' => $source)));
  provision_backend_invoke($destination, 'sql-cli < ' . $filepath);

  //@TODO: Sync files?  Or just use stage_file_proxy...

  //Get options
  $update = drush_get_option('no-update') == FALSE;
  $revert = drush_get_option('no-revert') == FALSE;
  $cache = drush_get_option('no-cache') == FALSE;

  // update db, unless option is false.
  if ($update){
    provision_backend_invoke($destination, 'updb');
  }
  
  // Revert All Features, unless option is false
  if ($revert){
    provision_backend_invoke($destination, 'features-revert-all');
  }

  // Clear the whole cache, unless option is false
  if ($cache){
    provision_backend_invoke($destination, 'cc all');
  }
  
  // Verify the site.
  provision_backend_invoke($destination, 'provision-verify');

  //Resume cron
  // @TODO: how to get hostmaster site name?  @see drush_provision_post_hostmaster_migrate()
  // they use drush_get_option('site_name')
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}

/**
 * drush_sql_sync_init() does what we want!
 */
function drush_provision_provision_devshop_sync_init($source = NULL, $destination = NULL) {
  return drush_sql_sync_init($source, $destination);
}