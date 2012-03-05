<?php

/**
 * Implements the provision-devshop-deploy command.
 *
 * @TODO: This command needs to take two aliases as arguments.
 *  Currently, the syntax is:
 *  $  drush @target pds @source
 *  
 *  Which feels backwards when you look at sql-sync
 *  $  drush sql-sync @source @target
 *
 */
function drush_provision_provision_devshop_sync() {
  $site_path = d()->platform->root;
  $target = d()->name;
  
  //Get source alias
  //@TODO: better way?  see rsync command
  $args = drush_get_arguments();
  $source = $args[1];   //$args[0] is always the command
  $source_name = substr($source, 1);  //Without @
  
  if (!$source){
    // @TODO: Check if alias exists?
    return drush_log(dt('Source not found.  You must enter a source alias to sync from'), 'error');
  }
  
  // @TODO: Pause Hostmaster (Mainly for the git pull task)

  // Pull latest version of db from live (this also creates a backup for rollbacks)
  // @TODO: figure out how to save temp files better.  See sql-sync
  drush_log(dt('Dumping sql from &alias...', array('&alias' => $source)));
  $filepath = "~/backups/$source_name.sql";
  $data = array(
    'result-file' => $filepath,
  );

  // Dump the source database
  provision_backend_invoke($source, 'sql-dump', array(), $data);

  // Prompt to drop database
  if (drush_prompt(dt('Drop the database for &alias?  WARNING: "Drop" means Delete all data!', array('&alias' => $source)))) {
    provision_backend_invoke($target, 'sql-drop');
  }
  
  // Backport live db to test
  provision_backend_invoke($target, 'sql-cli < ' . $filepath);
  
  // Run provision-devshop-revert
  if (drush_get_option('revert')) {
    provision_backend_invoke($target, 'provision-devshop-revert');
  }
  
  
  //@TODO: Sync files
}

