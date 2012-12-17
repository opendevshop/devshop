<?php
/**
 * @file pull.devshop.provision.inc
 * Contains code for the provision-devshop-pull command.
 */

/**
 * Pre provision-devshop-pull
 */
function drush_devshop_provision_pre_provision_devshop_pull(){
  devshop_provision_pre_flight();
}

/**
 * Implements the provision-devshop-pull command.
 */
function drush_devshop_provision_provision_devshop_pull() {
  
  // Get environments from argument (Inspired by function drush_pm_download())
  if (!$environments = _convert_csv_to_array(func_get_args())) {
    return drush_set_error(DRUSH_FRAMEWORK_ERROR, 'You must specify at least one environment to pull.');
  }
  
  foreach($environments as $platform_name){
    // Must specify environment to pull
    if (empty($platform_name)){
      drush_set_error(DRUSH_FRAMEWORK_ERROR, 'You must specify an environment to pull.');
      continue;
    }
    
    drush_log(dt('[DEVSHOP] Provision DevShop Pull started...'), 'ok');
    
    // Get platform & site object
    $project_name = d()->project_name;
    $platform_alias = '@' . implode('_', array('platform', $project_name, $platform_name));
    $platform = d($platform_alias);
    
    $site_alias = '@' . $platform_name . '.' . d()->base_url;
    $site = d($site_alias);
  
    // Find repo path
    $repo_path = $platform->root;
    
    // Ensure it's a git repo.
    provision_git_is_repo($repo_path);
    
    //Get options
    $update = drush_get_option('update');
    $revert = drush_get_option('revert');
    $cache = drush_get_option('cache');
  
    // Pull latest version of site
    //Execute git pull --rebase
    if (drush_shell_cd_and_exec($repo_path, 'git pull --rebase')) {
      drush_log(dt('[DEVSHOP] Git repository pulled.', array('!path' => $repo_path)), 'ok');
      $output = drush_shell_exec_output();
      drush_log(implode("\n", drush_shell_exec_output()), 'ok');
    }
    else {
      drush_set_error('DRUSH_PROVISION_GIT_PULL_FAILED', dt("Git pull failed in !path.\nThe specific errors are below:\n!errors", array('!path' => $repo_path, '!errors' => implode("\n", drush_shell_exec_output()))));
      continue;
    }
    
    // If no new files were detected... skip out.
    if (count($output) == 1) {
      drush_log('[DEVSHOP] No changes detected. Nothing else needs to be done', 'ok');
      continue;
    }
    
    // Verify the platform.     
    provision_backend_invoke($platform_alias, 'provision-verify');
    
    // update db, unless option is false.
    if ($update){
      drush_log(dt('[DEVSHOP] Updating database...'), 'ok');
      provision_backend_invoke($site_alias, 'updb');
    }
    else {
      drush_log(dt('[DEVSHOP] Skipped updating database...'), 'ok');
    }
    
    // Revert All Features, unless option is false
    if ($revert){
      drush_log(dt('[DEVSHOP] Reverting all features...'), 'ok');
      provision_backend_invoke($site_alias, 'features-revert-all');
    }
    else {
      drush_log(dt('[DEVSHOP] Skipped reverting all features...'), 'ok');
    }
    
    // Clear the whole cache, unless option is false
    // Seriously, lets do this twice.  Go Drupal!
    if ($cache){
      drush_log(dt('[DEVSHOP] Clearing all caches...'), 'ok');
      provision_backend_invoke($site_alias, 'cc all');
      provision_backend_invoke($site_alias, 'cc all');
    }
    else {
      drush_log(dt('[DEVSHOP] Skipped clearing all caches...'), 'ok');
    }
  }
}

