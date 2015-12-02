<?php
/**
 * @file deploy.devshop.provision.inc
 * Contains code for the provision-devshop-deploy command.
 */

use GitWrapper\GitWrapper;
use GitWrapper\GitException;

/**
 * Pre provision-devshop-deploy
 */
function drush_devshop_provision_pre_provision_devshop_deploy($git_ref = ''){

  if (d()->type == 'site') {
    if (empty(d()->environment)) {
      return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', 'This site is not a part of a project. You cannot use this command.');
    }
  }
  else {
    return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', 'provision-devshop-deploy must be run on a site context.');
  }

  // Verify that the branch or tag exists
  if (empty($git_ref)) {
    return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', 'You must specify a valid branch or tag.');
  }

  $project_alias = '@project_' . d()->project;
  $project = d($project_alias);

  if (!isset($project->project['settings']['git']['refs'][$git_ref])) {
    $drush_command = "drush $project_alias provision-verify";
    return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', "Branch or tag '$git_ref' not found. Try running '$drush_command' to fetch new remote branches or tags.");
  }
}

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_deploy($git_ref = '')
{
  $environment = (object) d()->environment;
  $project_name = d()->project;
  $project = d("@{$project_name}")->project;
  $desired_ref_type = $project->settings->git['refs'][$git_ref];

  try {

    $wrapper = new GitWrapper();

    $git = $wrapper->workingCopy($environment->repo_root);
    chdir($git->getDirectory());

    // Stash any changes? Not sure if we should do this anymore...
    // $git->command('stash');

    // Fetch
    drush_log('git fetch --all', 'devshop_command');
    $git->fetchAll();
    drush_log($git->getOutput(), 'devshop_ok');

    // Checkout the chosen ref
    drush_log("git checkout {$git_ref}", 'devshop_command');
    $git->checkout($git_ref);
    $git_checkout_output = $git->getOutput();
    drush_log($git_checkout_output, 'devshop_ok');

    $git_checkout_output_lines = explode("\n", $git_checkout_output);
    drush_log($git_checkout_output, 'devshop_ok');

    // Run Git Pull
    if ($desired_ref_type == 'branch') {

      drush_log("git pull", 'devshop_command');
      $git->pull();
      $git_pull_output = $git->getOutput();
      drush_log($git_pull_output, 'devshop_ok');

      // If on a branch and there are no changes it says "Already up-to-date."
      if ($git_pull_output == trim('Already up-to-date.')) {
        drush_set_context('devshop_block_deploy_hooks', TRUE);
        return;
      }
    }
    // If on a tag, and there are no changes, it outputs one line with the current ref.
    elseif ($desired_ref_type == 'tag' && count($git_checkout_output_lines) == 1) {
      drush_set_context('devshop_block_deploy_hooks', TRUE);
      return;
    }

    drush_log('git status', 'devshop_command');
    $git->status();
    drush_log($git->getOutput(), 'devshop_ok');

  } catch (GitException $e) {
    drush_log("Git Exception", 'devshop_command');
    drush_log($git->getOutput(), 'devshop_ok');
    drush_log($e->getMessage(), 'devshop_error');
    return drush_set_error('DEVSHOP_GIT_ERROR');
  }
}

/**
 * Post provision-devshop-deploy
 */
function drush_devshop_provision_post_provision_devshop_deploy($git_ref = '') {

  // @TODO: Remove this! This is not a full hostmaster task, so project data
  // is not being saved in hostmaster.
  // Verify the site.
  drush_log(dt('[DEVSHOP] Verifying the site.'), 'ok');
  provision_backend_invoke(d()->name, 'provision-verify');

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

