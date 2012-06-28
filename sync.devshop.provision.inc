<?php

require_once DRUSH_BASE_PATH . '/commands/sql/sync.sql.inc';

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
  drush_log(dt('DevShop: Dropping tables for destination'), 'ok');
  provision_backend_invoke($destination, 'sql-drop');
  
  // Run provision-devshop-pull if requested
  if (drush_get_option('pull')) {
    if (!provision_backend_invoke($destination, 'provision-git-pull')){
      return;
    }
  }
  
  // Sync the databases
  $timestamp = date("Ymd-His");
  //@TODO I know there is a better way...  Don't remember where to find it.
  $file_path = drush_server_home() . "/backups/devshop-sync/";
  $file_name = "$source_name-$timestamp.sql";
  $file = $file_path . $file_name;
  
  //Create backup folder if does not exist
  //@TODO: DO THIS ON REMOTE AS WELL
  if (!file_exists($file_path)){
    provision_file()->mkdir($file_path)
      ->succeed('Created devshop-sync backups folder: ' . $file_path)
     ->fail('Could not create the folder: ' . $file_path)
     ->status();
  }
  
  $options = array(
    'source-dump' => $file,
    'target-dump' => $file,
    'result-file' => $file,
    //@TODO: get this working.  Have to figure out proper code to extract gz into SQL Import
    //'gzip' => TRUE,  
  );
  
  //Source and target dump options aren't working here... @TODO: figure out why!
  //provision_backend_invoke('@hostmaster', 'sql-sync', array($source, $destination), $options);
  //drush_invoke('sql-sync', $source, $destination);
  
  // SQL Dump
  drush_log(dt('DevShop: Creating SQL dump from !source', array('!source' => $source)), 'ok');
  provision_backend_invoke($source, 'sql-dump', array(), $options);

  // COPY SQL file if it doesn't exist (which means a remote source)
  // @TODO: Determine if source is remote instead of just checking if file is there.
  if (!file_exists($file)){
    // Rsync to this machine.
    drush_log('Copying backup from remote server', 'ok');
    
    if (provision_backend_invoke('server_master', 'rsync', array($source . ':' . $file_path, $file_path))) {
      drush_log('SQL transfered', 'ok');
    } else {
      return drush_log(dt('Failed to copy the backup from the remote server'), 'error');
    }
  }
  
  // SQL Import
  // provision_backend_invoke() does not work here.  Doesn't like piping
  drush_log(dt('DevShop: Importing SQL into !destination', array('!destination' => $destination)), 'ok');
  exec("drush $destination sql-cli < $file");

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
function drush_provision_devshop_sync_init($source = NULL, $destination = NULL) {
  return drush_sql_sync_init($source, $destination);
}
