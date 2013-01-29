<?php

require_once DRUSH_BASE_PATH . '/commands/sql/sync.sql.inc';


/**
 * Implements drush_hook_init
 * drush_sql_sync_init() does what we want!
 */
function drush_provision_devshop_sync_init($source = NULL, $destination = NULL) {
  // Source and destination here are just platform names.  We need the full
  // alias.
  $source = $source .'.'. $d->base_url;
  $destination = $destination .'.'. $d->base_url;
  return drush_sql_sync_init($source, $destination);
}

/**
 * Pre provision-devshop-pull
 */
function drush_devshop_provision_pre_provision_devshop_sync(){
  devshop_provision_pre_flight();
}

/**
 * Pre provision-devshop-pull
 */
function drush_devshop_provision_provision_devshop_sync_validate($source = NULL, $destination = NULL){
  drush_set_arguments(array($source, $destination));
}

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_sync($source = NULL, $destination = NULL) {
  drush_log(dt('[DEVSHOP] Provision DevShop Sync started...'));

  // Get source name
  if ($source){
    $source_name = substr($source, 1);  // Without @
  } else {
    $source = drush_get_option('source');
    $source_name = substr($source, 1); // Without @
  }

  // Fail if no source
  if (!$source_name){
    return drush_log(dt('[DEVSHOP] Source not found.  You must enter a source alias to sync from'), 'error');
  } else {
    $source_site_alias = $source .'.'. d()->base_url;
  }

  // Get destination name
  if ($destination){
    $destination_name = substr($destination, 1); // Without @
  } else {
    //If $destination as an argument is not present it is the target name
    $destination = drush_get_option('destination');
    $destination_name = substr($destination, 1); // Without @
  }

  // Fail if no destination
  if (!$destination_name){
    return drush_log(dt('[DEVSHOP] Destination not found.  You must enter a destination alias to sync from'), 'error');
  } else {
    $destination_site_alias = $destination .'.'. d()->base_url;
  }


   // Run provision-devshop-pull if requested
  if (drush_get_option('pull')) {
    if (!provision_backend_invoke($destination_site_alias, 'provision-git-pull')){
      return;
    }
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped pulling code for destination...'), 'ok');
  }

  if (drush_get_option('database')){

  // Drop the database
    drush_log(dt('[DEVSHOP] Dropping tables on destination'), 'ok');
    provision_backend_invoke($destination_site_alias, 'sql-drop');


    drush_log(dt('[DEVSHOP] Syncing databases...'), 'ok');
  // Sync the databases
    $timestamp = date("Ymd-His");
  //@TODO I know there is a better way...  Don't remember where to find it.
    $file_path = drush_server_home() . "/backups/devshop-sync/";
    $file_name = "$source_name-$timestamp.sql";
    $file = $file_path . $file_name;
  // Create backup folder if does not exist
  // @TODO: DO THIS ON REMOTE AS WELL
    if (!file_exists($file_path)){
      provision_file()->mkdir($file_path)
      ->succeed('[DEVSHOP] Created devshop-sync backups folder: ' . $file_path)
      ->fail('[DEVSHOP] Could not create the folder: ' . $file_path)
      ->status();
    }

    $options = array(
      'source-dump' => $file,
      'target-dump' => $file,
      'result-file' => $file,
    //@TODO: get this working.  Have to figure out proper code to extract gz into SQL Import
    //'gzip' => TRUE,
      );

  // Source and target dump options aren't working here... @TODO: figure out why!
  //provision_backend_invoke('@hostmaster', 'sql-sync', array($source, $destination), $options);
  //drush_invoke('sql-sync', $source, $destination);

  // SQL Dump
    drush_log(dt('[DEVSHOP] Creating SQL dump from !source', array('!source' => $source_site_alias)), 'ok');
    provision_backend_invoke($source_site_alias, 'sql-dump', array(), $options);

  // COPY SQL file if it doesn't exist (which means a remote source)
  // @TODO: Determine if source is remote instead of just checking if file is there.
    if (!file_exists($file)){
    // Rsync to this machine.
      drush_log('Copying backup from remote server', 'ok');

      if (provision_backend_invoke('server_master', 'rsync', array($source_site_alias . ':' . $file_path, $file_path))) {
        drush_log('SQL transfered', 'ok');
      } else {
        return drush_log(dt('[DEVSHOP] Failed to copy the backup from the remote server'), 'error');
      }
    }

  // SQL Import
  // provision_backend_invoke() does not work here.  Doesn't like piping
    drush_log(dt('[DEVSHOP] Importing SQL into !destination', array('!destination' => $destination_site_alias)), 'ok');
    exec("drush @$destination_site_alias sql-cli < $file");
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped syncing databases...'), 'ok');
  }

  // Sync files via rsync
  if (drush_get_option('files')) {
     drush_log(dt('[DEVSHOP] Syncing file contents from !source to !destination', array('!source' => $source_site_alias, '!destination' => $destination_site_alias)), 'ok');


     provision_backend_invoke(d()->server->name, 'rsync', array("@$source_site_alias:sites/$source_site_alias/files/", "@$destination_site_alias:sites/$destination_site_alias/files/"));
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped syncing files...'), 'ok');
  }

  // @TODO: Put these in a POST command hook.  The other tasks use them too.
  // Get options
  $update = drush_get_option('update');
  $revert = drush_get_option('revert');
  $cache = drush_get_option('cache');

  $site = "@$destination_site_alias";

  // update db, unless option is false.
  if ($update){
    drush_log(dt('[DEVSHOP] Updating database...'), 'ok');
    provision_backend_invoke($site, 'updb');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped updating database...'), 'ok');
  }

  // Revert All Features, unless option is false
  if ($revert){
    drush_log(dt('[DEVSHOP] Reverting all features...'), 'ok');
    provision_backend_invoke($site, 'features-revert-all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped reverting all features...'), 'ok');
  }

  // Clear the whole cache, unless option is false
  // Seriously, lets do this twice.  Go Drupal!
  if ($cache){
    drush_log(dt('[DEVSHOP] Clearing all caches...'), 'ok');
    provision_backend_invoke($site, 'cc all');
    provision_backend_invoke($site, 'cc all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped clearing all caches...'), 'ok');
  }
}
