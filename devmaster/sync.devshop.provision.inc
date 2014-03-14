<?php

require_once DRUSH_BASE_PATH . '/commands/sql/sync.sql.inc';


/**
 * Implements drush_hook_init
 * drush_sql_sync_init() does what we want!
 */
function drush_provision_devshop_sync_init($source = NULL, $destination = NULL) {

  // Source and destination here are just platform names.  We need the full
  // alias.  Source can also be another alias.
  if (strpos($source, '@') === 0){
    $source = substr($source, 1);
  }
  else {
    $source = $source . '.' . d()->base_url;
  }
  $destination = $destination . '.' . d()->base_url;
  return drush_sql_sync_init($source, $destination);
}

/**
 * Pre provision-devshop-sync
 */
function drush_devshop_provision_pre_provision_devshop_sync() {
  devshop_provision_pre_flight();
}

/**
 * Pre provision-devshop-sync
 */
function drush_devshop_provision_provision_devshop_sync_validate($source = NULL, $destination = NULL) {
}

/**
 * Implements the provision-devshop-sync command.
 *
 * Now expects environment names for source and destination
 */
function drush_devshop_provision_provision_devshop_sync($source = NULL, $destination = NULL) {
  // Source and destination here are just platform names.  We need the full
  // alias.  Source can also be another alias.
  if (strpos($source, '@') === 0){
    $source = substr($source, 1);
  }
  else {
    $source = $source . '.' . d()->base_url;
  }
  $destination = $destination . '.' . d()->base_url;

  // Aliases
  $source_alias = "@$source";
  $destination_alias = "@$destination";

  drush_log(dt("[DEVSHOP] Provision DevShop Sync started from $source to $destination... "), 'ok');
  $base_url = d()->base_url;
  
  // Fail if base url
  if (empty($base_url)) {
    return drush_log(dt('[DEVSHOP] Your project doesn\'t seem to have the base_url saved.  Run project verification.'), 'error');
  }
  
  // Fail if no source
  if (empty($source) && !($source = drush_get_option('source'))) {
    return drush_log(dt('[DEVSHOP] Source not found.  You must enter a source alias to sync from'), 'error');
  }
  else {
    drush_log(dt('[DEVSHOP] Source found: ' . $source));
  }

  // Fail if no destination
  if (empty($destination) && !($destination = drush_get_option('destination'))) {
    return drush_log(dt('[DEVSHOP] Destination not found.  You must enter a source alias to sync to.'), 'error');
  }
  else {
    drush_log(dt('[DEVSHOP] Destination found: '. $destination));
  }

  // Run provision-devshop-pull if requested
  if (drush_get_option('pull')) {
    if (!provision_backend_invoke($destination, 'provision-git-pull')) {
      return;
    }
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped pulling code for destination...'), 'ok');
  }

  if (drush_get_option('database')) {
  // Drop the database
    drush_log(dt('[DEVSHOP] Dropping tables for destination database (!dest)', array('!dest' => $destination)), 'ok');
    provision_backend_invoke($destination, 'sql-drop');
    drush_log(dt('[DEVSHOP] Syncing databases...'), 'ok');
    
    // Sync the databases
    drush_backend_invoke('sql-sync', array($source_alias, $destination_alias));
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped syncing databases...'), 'ok');
  }

  // Sync files via rsync
  if (drush_get_option('files')) {
    drush_log(dt('[DEVSHOP] Syncing file contents from !source to !destination', array('!source' => $source, '!destination' => $destination)), 'ok');
    
    // Sync the files
    // @TODO: Use %files aliases?
    drush_backend_invoke('rsync', array("$source_alias:sites/$source/files/", "$destination_alias:sites/$destination/files/"));
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped syncing files...'), 'ok');
  }

  // @TODO: Put these in a POST command hook.  The other tasks use them too.
  // Get options
  $update = drush_get_option('update');
  $revert = drush_get_option('revert');
  $cache = drush_get_option('cache');

  // update db, unless option is false.
  if ($update) {
    drush_log(dt('[DEVSHOP] Updating database...'), 'ok');
    provision_backend_invoke($destination, 'updb');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped updating database...'), 'ok');
  }

  // Revert All Features, unless option is false
  if ($revert) {
    drush_log(dt('[DEVSHOP] Reverting all features...'), 'ok');
    provision_backend_invoke($destination, 'features-revert-all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped reverting all features...'), 'ok');
  }

  // Clear the whole cache, unless option is false
  // Seriously, lets do this twice.  Go Drupal!
  if ($cache) {
    drush_log(dt('[DEVSHOP] Clearing all caches...'), 'ok');
    provision_backend_invoke($destination, 'cc all');
    provision_backend_invoke($destination, 'cc all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped clearing all caches...'), 'ok');
  }
}
