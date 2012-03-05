<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_sync() {
  $site_path = d()->platform->root;
  $target = d()->name;
  
  $source = drush_get_option('source');
  
  if (!$source){
    return drush_log(dt('Source not found.  You must enter a source alias to sync from'), 'error');
  }
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO

  // Pull latest version of db from live (this also creates a backup for rollbacks)
  //@TODO: figure out how to save temp files.  See sql-sync
  $filename = "$source.sql";
  $filepath = "~/backups/$filename";
  $data = array(
    'result-file' => $filepath,
  );
  provision_backend_invoke($source, 'sql-dump', array(), $data);

  // Prompt to drop database
  if (drush_prompt(dt('Drop the database for &alias?  WARNING: "Drop" means Delete all data!', array('&alias' => $source)))) {
    provision_backend_invoke($target, 'sql-drop');
  }
  
  // Backport live db to test
  provision_backend_invoke($target, 'sql-cli < ' . $filepath);
  
  // Run provision-devshop-revert
  if (drush_prompt(dt('Run provision-devshop-revert on &alias?', array('&alias' => $source)))) {
    provision_backend_invoke($target, 'provision-devshop-revert');
  }
  
  
  //@TODO: Sync files
}

