<?php

require_once DRUSH_BASE_PATH . '/commands/sql/sync.sql.inc';


/**
 * Implements drush_hook_init
 * drush_sql_sync_init() does what we want!
 */
function drush_provision_devshop_sync_init($source = NULL, $destination = NULL) {
  // Source and destination here are just platform names.  We need the full
  // alias.
  $source = $source . '.' . d()->base_url;
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
  drush_set_arguments(array($source, $destination));
}

/**
 * Implements the provision-devshop-sync command.
 *
 * Now expects environment names for source and destination
 */
function drush_devshop_provision_provision_devshop_sync($source = NULL, $destination = NULL) {
  drush_log(dt('[DEVSHOP] Provision DevShop Sync started...'));
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
    $source_site = "$source.$base_url";
    $source_site_alias = $source_site;
    drush_log(dt('[DEVSHOP] Source found: ' . $source_site_alias));
  }

  // Fail if no destination
  if (empty($destination) && !($destination = drush_get_option('destination'))) {
    return drush_log(dt('[DEVSHOP] Destination not found.  You must enter a source alias to sync to.'), 'error');
  }
  else {
    $destination_site = "$destination.$base_url";
    $destination_site_alias = $destination_site;
    drush_log(dt('[DEVSHOP] Destination found: '. $destination_site_alias));
  }

  // Run provision-devshop-pull if requested
  if (drush_get_option('pull')) {
    if (!provision_backend_invoke($destination_site_alias, 'provision-git-pull')) {
      return;
    }
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped pulling code for destination...'), 'ok');
  }

  if (drush_get_option('database')) {
  // Drop the database
    drush_log(dt('[DEVSHOP] Dropping tables for destination database (!dest)', array('!dest' => $destination)), 'ok');
    provision_backend_invoke($destination_site_alias, 'sql-drop');
    drush_log(dt('[DEVSHOP] Syncing databases...'), 'ok');
    
    // Sync the databases
    drush_backend_invoke('sql-sync', array('@' . $source_site_alias, '@' . $destination_site_alias));
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped syncing databases...'), 'ok');
  }

  // Sync files via rsync
  if (drush_get_option('files')) {
    drush_log(dt('[DEVSHOP] Syncing file contents from !source to !destination', array('!source' => $source_site_alias, '!destination' => $destination_site_alias)), 'ok');
    
    // Sync the files
    drush_backend_invoke('rsync', array("@$source_site_alias:sites/$source_site/files/", "@$destination_site_alias:sites/$destination_site/files/"));
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
    provision_backend_invoke($destination_site_alias, 'updb');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped updating database...'), 'ok');
  }

  // Revert All Features, unless option is false
  if ($revert) {
    drush_log(dt('[DEVSHOP] Reverting all features...'), 'ok');
    provision_backend_invoke($destination_site_alias, 'features-revert-all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped reverting all features...'), 'ok');
  }

  // Clear the whole cache, unless option is false
  // Seriously, lets do this twice.  Go Drupal!
  if ($cache) {
    drush_log(dt('[DEVSHOP] Clearing all caches...'), 'ok');
    provision_backend_invoke($destination_site_alias, 'cc all');
    provision_backend_invoke($destination_site_alias, 'cc all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped clearing all caches...'), 'ok');
  }
}
