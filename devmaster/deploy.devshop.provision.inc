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
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_deploy() {
  // Get options
  $environment = drush_get_option('environment');
  $git_ref = drush_get_option('git_ref');

  // Get post deploy options
  $revert = drush_get_option('revert');
  $update = drush_get_option('update');
  $cache = drush_get_option('cache');
  $force = drush_get_option('force');

  // Find repo path
  $repo_path = d()->project['code_path'] . '/' . $environment;

  // @TODO: Add to a drush verify hook

  // Check that environment exists
  if (!isset(d()->project['environments'][$environment])) {
    return drush_set_error('DRUSH_ERROR', dt("Environment !env does not exist in project !proj.", array('!env' => $environment, '!proj' => d()->project['name'])));
  }

  // Verify the project so we have all the git refs.
  // @TODO! Hostmaster hook runs the git ref lookup! move that code to devshop provision
  //  provision_backend_invoke(d()->name, 'provision-verify');

  // Check that the git ref exists
  if (!isset(d()->project['settings']['git']['refs'][$git_ref])) {
    return drush_set_error('DRUSH_ERROR', dt("Git reference !git does not exist in the git remote !remote.", array('!git' => $git_ref, '!remote' => d()->project['git_url'])));
  }

  drush_log(dt('[DEVSHOP] Deploy: Git Ref "' . $git_ref . '" to Environment "' . $environment . '"'), 'ok');

  // Get platform & site object
  $project_name = d()->project_name;
  $platform_alias = '@' . implode('_', array('platform', $project_name, $environment));
  $platform = d($platform_alias);

  $site_alias = '@' . $environment . '.' . d()->project['base_url'];
  $site = d($site_alias);


  // Ensure it's a git repo.
  provision_git_is_repo($repo_path);

  // If reset is true, do a git reset --hard first.
  drush_shell_cd_and_exec($repo_path, 'git stash');
  drush_log(dt('[DEVSHOP] Git working copy stashed.'), 'ok');

  drush_shell_cd_and_exec($repo_path, "git fetch");
  drush_log(dt('[DEVSHOP] Git fetch called.'), 'ok');

  // Pull latest version of site
  if (drush_shell_cd_and_exec($repo_path, "git checkout $git_ref")) {
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

  drush_shell_cd_and_exec($repo_path, "git status");

}

