<?php
/**
 * @file deploy.devshop.provision.inc
 * Contains code for the provision-devshop-deploy command.
 */

use Symfony\Component\Process\Process;
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
  $project = (object) d($project_alias)->project;

  if (!isset($project->settings['git']['refs'][$git_ref])) {
    $drush_command = "drush $project_alias provision-verify";
    return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', "Branch or tag '$git_ref' not found. Try running '$drush_command' to fetch new remote branches or tags.");
  }
}

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_deploy($git_ref = '')
{
  $project_name = d()->project;
  $project = (object) d("@project_{$project_name}")->project;
  $environment = (object) $project->environments[d()->environment];
  $desired_ref_type = $project->settings['git']['refs'][$git_ref];

  try {

    $wrapper = new GitWrapper();
    $wrapper->streamOutput();
    $git = $wrapper->workingCopy($environment->repo_root);
    drush_log('[Current Working Directory]' . $git->getDirectory(), 'devshop_log');

    // Stash any changes? Not sure if we should do this anymore...
    // $git->command('stash');

    // Fetch
    devshop_drush_process("git fetch --all", $environment->repo_root, dt('DevShop Deploy'));

    // Checkout the chosen ref
    $git_checkout_output = devshop_drush_process("git checkout {$git_ref}", $environment->repo_root, dt('DevShop Deploy'));
    $git_checkout_output_lines = explode("\n", $git_checkout_output);

    // Run Git Pull
    if ($desired_ref_type == 'branch') {

      $git_pull_output = devshop_drush_process("git pull", $environment->repo_root, dt('DevShop Deploy'));
      devshop_drush_process("git submodule update --recursive ", $environment->repo_root, dt('DevShop Deploy'));

      // If on a branch and there are no changes it says "Already up-to-date."
      if (strpos($git_pull_output, 'Already up-to-date.') !== FALSE) {
        drush_set_context('devshop_block_deploy_hooks', TRUE);
      }
    }
    // If on a tag, and there are no changes, it outputs one line with the current ref.
    elseif ($desired_ref_type == 'tag' && count($git_checkout_output_lines) == 1) {
      drush_set_context('devshop_block_deploy_hooks', TRUE);
    }

    devshop_drush_process("git smattus", $environment->repo_root, dt('DevShop Deploy'));

  } catch (GitException $e) {
    drush_log("[DevShop Deploy] Git Exception", 'devshop_command');
    drush_log($e->getMessage(), 'devshop_error');
    drush_log($e->getMessage(), 'error');
    return FALSE;
  }
}

/**
 * Post provision-devshop-deploy
 */
function drush_devshop_provision_post_provision_devshop_deploy($git_ref = '') {

  if (drush_get_context('devshop_block_deploy_hooks')){
    drush_log(dt('Skipping deploy hooks. No changes detected.'), 'devshop_info');
    return;
  }

  // Get post deploy options
  $revert = drush_get_option('revert');
  $update = drush_get_option('update');
  $cache = drush_get_option('cache');

  $project_name = d()->project;
  $project = (object) d("@project_{$project_name}")->project;
  $environment = (object) $project->environments[d()->environment];
  $desired_ref_type = $project->settings['git']['refs'][$git_ref];

  // Ensure drush_alias exists. Not sure why it was missing for me.
  if (empty($environment->drush_alias)) {
    $environment->drush_alias = d()->name;
  }

  $commands = array();

  drush_log("[{$project_name}] {$environment->name}: " . dt('Running deploy hooks.'), 'notice');

  // Built in Hooks
  if ($update) {
    $commands[] = "drush {$environment->drush_alias} updatedb --yes";
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped updating database...'), 'ok');
  }


  // Revert All Features, unless option is false
  if ($revert) {
    $commands[] = "drush {$environment->drush_alias} features-revert-all --yes";
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped reverting all features...'), 'ok');
  }

  // Clear the whole cache, unless option is false
  if ($cache) {
    if (drush_drupal_major_version(d()->root) == 8) {
      $commands[] = "drush {$environment->drush_alias} cache-rebuild";
    }
    else {
      $commands[] = "drush {$environment->drush_alias} cache-clear all";
    }
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped clearing all caches...'), 'ok');
  }

  foreach ($commands as $command) {
    devshop_drush_process($command, $environment->repo_root, 'DevShop Deploy Hook');
  }
}

