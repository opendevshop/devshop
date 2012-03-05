<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_sync($source = NULL, $destination = NULL) {

  //Get source name for filenames
  $source_name = substr($source, 1);  //Without @
  if (!$source){
    return drush_log(dt('Source not found.  You must enter a source alias to sync from'), 'error');
  }
  
  // @TODO: Pause Hostmaster (Mainly for the git pull task)

  // Prompt to drop database
  if (drush_confirm(dt('Drop the database for &alias?  WARNING: "Drop" means Delete all data!', array('&alias' => $destination)))) {
    provision_backend_invoke($destination, 'sql-drop');
  }
  
  // Sync the databases
  // @TODO: Prompt here, too.  If a user is running this command, they should be warned the same as sql-sync
  $timestamp = date("Ymd-His");
  $filepath = "~/backups/$source_name-$timestamp.sql";
  $options = array(
    'source-dump' => $filepath,
    'target-dump' => $filepath,
    'gzip' => TRUE,
  );
  provision_backend_invoke($destination, 'sql-sync', array($source, $destination), $options);

  //@TODO: Sync files

  // Run provision-devshop-revert
  if (drush_get_option('revert')) {
    provision_backend_invoke($destination, 'provision-devshop-revert');
  }

}

/**
 * drush_sql_sync_init() does what we want!
 */
function drush_provision_provision_devshop_sync_init($source = NULL, $destination = NULL) {
  return drush_sql_sync_init($source, $destination);
}