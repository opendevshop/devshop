<?php
/**
 * @file deploy.devshop.provision.inc
 * Contains code for the provision-devshop-deploy command.
 */

/**
 * Pre provision-devshop-deploy
 */
function drush_devshop_provision_pre_provision_devshop_deploy($git_ref = ''){

  if (d()->type == 'site') {
    if (empty(d()->environment)) {
      return drush_set_error(DEVSHOP_FRAMEWORK_ERROR, 'This site is not a part of a project. You cannot use this command.');
    }
  }
  else {
    return drush_set_error(DEVSHOP_FRAMEWORK_ERROR, 'provision-devshop-deploy must be run on a site context.');
  }

  // Verify that the branch or tag exists
  if (empty($git_ref)) {
    return drush_set_error(DEVSHOP_FRAMEWORK_ERROR, 'You must specify a valid branch or tag.');
  }

  $project_alias = '@project_' . d()->project;
  $project = d($project_alias);

  if (!isset($project->project['settings']['git']['refs'][$git_ref])) {
    $drush_command = "drush $project_alias provision-verify";
    return drush_set_error(DEVSHOP_FRAMEWORK_ERROR, "Branch or tag '$git_ref' not found. Try running '$drush_command' to fetch new remote branches or tags.");
  }
}

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_deploy($git_ref = '') {
  // Get info
  $environment = (object) d()->environment;
  $project_name = d()->project;
  $project = (object) d("@project_{$project_name}")->project;

  drush_log("[{$project_name}] Deploying '{$git_ref}' to '{$environment->name}'.", 'ok');


  $tag_or_branch = $environment->git_ref_type;

  // Find repo path
  $repo_path = $project->code_path . '/' . $environment->name;

  // Ensure it's a git repo.
  provision_git_is_repo($repo_path);

  // If reset is true, do a git reset --hard first.
  drush_shell_cd_and_exec($repo_path, 'git stash');
  drush_log(dt('[DEVSHOP] Git working copy stashed.'), 'ok');

  drush_shell_cd_and_exec($repo_path, "git fetch");
  drush_log(dt('[DEVSHOP] Git fetch called.'), 'ok');

  // Check the desired git_ref
  if (drush_shell_cd_and_exec($repo_path, "git checkout $git_ref")) {
    drush_log(dt("[DEVSHOP] Git {$tag_or_branch} {$git_ref} checked out to {$environment->name} environment, located at {$repo_path}."), 'ok');
    $output = drush_shell_exec_output();
    drush_log(implode("\n", $output), 'ok');

    // Run Git Pull
    if ($tag_or_branch == 'branch') {
      drush_shell_cd_and_exec($repo_path, "git pull");
      $output = drush_shell_exec_output();

      drush_log(implode("\n", $output), 'ok');

      // If on a branch and there are no changes it says "Already up-to-date."
      if ($output[0] == 'Already up-to-date.') {
        drush_set_context('devshop_block_deploy_hooks', TRUE);
        return;
      }
    }
    // If on a tag, and there are no changes, it outputs one line with the current ref.
    elseif ($tag_or_branch == 'tag' && count($output) == 1) {
      drush_set_context('devshop_block_deploy_hooks', TRUE);
      return;
    }
  }
  else {
    drush_set_error('DRUSH_PROVISION_GIT_PULL_FAILED', dt("Git pull failed in !path.\nThe specific errors are below:\n!errors", array(
          '!path' => $repo_path,
          '!errors' => implode("\n", drush_shell_exec_output())
        )));
    return;
  }
  drush_shell_cd_and_exec($repo_path, "git status");
  $output = drush_shell_exec_output();

  drush_log('----------------------------------------', 'ok');
  drush_log('$ git status', 'ok');
  drush_log(implode("\n", $output), 'ok');
  drush_log('----------------------------------------', 'ok');

}

/**
 * Post provision-devshop-deploy
 */
function drush_devshop_provision_post_provision_devshop_deploy($git_ref = '') {

  if (drush_get_context('devshop_block_deploy_hooks')){
    drush_log(dt('[DEVSHOP] Skipping deploy hooks. No changes detected.'), 'ok');
    return;
  }
  else {
    drush_log(dt('[DEVSHOP] Preparing to run deploy hooks...'), 'ok');
  }

  // Get post deploy options
  $revert = drush_get_option('revert');
  $update = drush_get_option('update');
  $cache = drush_get_option('cache');

  $environment = (object) d()->environment;
  $project_name = d()->project;

  drush_log("[{$project_name}] {$environment->name}: " . dt('Running deploy hooks.'), 'notice');

  // Built in Hooks
  if ($update) {
    drush_log("[{$project_name}] {$environment->name}:" . dt('Running updb...'), 'notice');
    provision_backend_invoke(d()->name, 'updb');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped updating database...'), 'ok');
  }


  // Revert All Features, unless option is false
  if ($revert) {
    drush_log(dt('[DEVSHOP] Reverting all features...'), 'ok');
    provision_backend_invoke(d()->name, 'features-revert-all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped reverting all features...'), 'ok');
  }

  // Clear the whole cache, unless option is false
  // Seriously, lets do this twice.  Go Drupal!
  if ($cache) {
    drush_log(dt('[DEVSHOP] Clearing all caches...'), 'ok');
    provision_backend_invoke(d()->name, 'cc all');
    provision_backend_invoke(d()->name, 'cc all');
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped clearing all caches...'), 'ok');
  }
}

