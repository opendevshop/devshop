<?php
/**
 * @file deploy.devshop.provision.inc
 * Contains code for the provision-devshop-deploy command.
 */

/**
 * Pre provision-devshop-deploy
 */
function drush_devshop_provision_pre_provision_devshop_deploy(){
  devshop_provision_pre_flight();
}

/**
 * Implements the provision-devshop-pull command.
 */
function drush_devshop_provision_provision_devshop_deploy($environment, $git_ref) {
//  $args = func_get_args();

  // Detect the string argument and change it into the list of args.
  // @TODO: Aegir, why do we need this?  When run from drush, $args contain all
  //  arguments.
  // Detect spaces
  if (strpos($args[0], ' ') !== FALSE){
    // if there is a list of environments, break it up and add it to a new $args
    $args = explode(' ', $args[0]);
  }

  // Get global options
  $revert = drush_get_option('revert');
  $update = drush_get_option('update');
  $cache = drush_get_option('cache');
  $force = drush_get_option('force');
  
  drush_log(dt('[DEVSHOP] Provision DevShop Deploy: Git Ref ' . $git_ref . ' to Environment ' . $environment), 'ok');

  // Get platform & site object
  $project_name = d()->project_name;
  $platform_alias = '@' . implode('_', array('platform', $project_name, $environment));
  $platform = d($platform_alias);

  $site_alias = '@' . $environment . '.' . d()->project['base_url'];
  $site = d($site_alias);

  // Find repo path
  $repo_path = $platform->root;

  // Ensure it's a git repo.
  provision_git_is_repo($repo_path);

  // If reset is true, do a git reset --hard first.
  drush_shell_cd_and_exec($repo_path, 'git stash');
  drush_log(dt('[DEVSHOP] Git working copy stashed.'), 'ok');

  // Pull latest version of site
  if (drush_shell_cd_and_exec($repo_path, "git checkout $git_ref && git pull")) {
    drush_log(dt("[DEVSHOP] Git Ref $git_ref checked out to $environment environment, located at $repo_path."), 'ok');
    $output = drush_shell_exec_output();
    drush_log(implode("\n", drush_shell_exec_output()), 'ok');
  }
  else {
    drush_set_error('DRUSH_PROVISION_GIT_PULL_FAILED', dt("Git pull failed in !path.\nThe specific errors are below:\n!errors", array('!path' => $repo_path, '!errors' => implode("\n", drush_shell_exec_output()))));
    return;
  }

  // If no new files were detected... and force is false then skip out.
  if (count($output) == 1 && !$force) {
    drush_log('[DEVSHOP] No changes detected. Nothing else needs to be done', 'ok');
    return;
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

