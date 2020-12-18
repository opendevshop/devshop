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
      return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', 'This site is not a part of a project. You cannot use this command.');
    }
  }
  else {
    return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', 'provision-devshop-deploy must be run on a site context.');
  }

  // Verify that the branch or tag exists
  if (empty($git_ref) && empty(d()->platform->git_ref)) {
    return drush_set_error('DEVSHOP_FRAMEWORK_ERROR', 'You must specify a valid branch or tag.');
  }
}

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_deploy($git_ref = '')
{
  if (empty($git_ref)) {
    $git_ref = d()->platform->git_ref;
  }
//
//    // Stash any changes? Not sure if we should do this anymore...
//    // $git->command('stash');
//
//    // Fetch
//    drush_log('[Current Working Directory]' . d()->platform->repo_path, 'p_log');
//
//    provision_process("git fetch origin", d()->platform->repo_path, dt('DevShop Deploy: Fetch Changes'));
//    provision_process("git status", d()->platform->repo_path, dt('DevShop Deploy: Git Status'));
//
//    // Checkout the chosen ref
//    $git_checkout_output = provision_process("git checkout {$git_ref}", d()->platform->repo_path, dt('DevShop Deploy: Checkout Code'));
//    $git_checkout_output_lines = explode("\n", $git_checkout_output);
//
//    // @TODO: Get the correct git ref type.
//    $git_ref_type = devshop_environment_get_git_ref_type(d()->platform->repo_path);
//
//    drush_log("[Git $git_ref_type to Deploy] $git_ref", 'p_log');
//
//    // If the current working copy was detected to be a branch, update the code
//    // to the exact reference of the current remote.
//    // This is the better than using git pull because it can handle force pushed branches.
//    if ($git_ref_type == 'branch') {
//      provision_process("git reset --hard origin/{$git_ref}", d()->platform->repo_path, dt('DevShop Deploy: Update code to match upstream branch.'));
//    }
//
//    // Run a submodule update and init.
//    provision_process("git submodule update --init --recursive --force ", d()->platform->repo_path, dt('DevShop Deploy: Init Submodules'));
//
//    provision_process("git status", d()->platform->repo_path, dt('DevShop Deploy: Git Status'));
}

/**
 * Post provision-devshop-deploy
 */
function drush_devshop_provision_post_provision_devshop_deploy($git_ref = '') {

  if (empty($git_ref)) {
    $git_ref = d()->platform->git_ref;
  }

  // Get post deploy options
  $revert = drush_get_option('revert');
  $update = drush_get_option('update');
  $cache = drush_get_option('cache');
  $composer = drush_get_option('composer', TRUE);

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
  if ($composer) {
    $composer_install_command = drush_get_option('provision_composer_install_command', 'composer install --no-interaction --no-progress --no-dev');
    if (file_exists(d()->platform->repo_path . '/composer.json')) {
      $repo_path = d()->platform->repo_path;
      $commands[] = "cd {$repo_path} && {$composer_install_command}";
    }
    elseif (file_exists($environment->root . '/composer.json')) {
      $commands[] = "cd {$environment->root} && {$composer_install_command}";
    }
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped running composer install...'), 'ok');
  }

  // Built in Hooks
  if ($update) {
    $commands[] = "drush {$environment->drush_alias} updatedb --yes";
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped updating database...'), 'ok');
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

  // Revert All Features, unless option is false
  if ($revert) {
    $commands[] = "drush {$environment->drush_alias} features-revert-all --yes";
  }
  else {
    drush_log(dt('[DEVSHOP] Skipped reverting all features...'), 'ok');
  }

  foreach ($commands as $command) {
    $output = provision_process($command, d()->platform->repo_path, 'DevShop Deploy Hook');

    // Detect common errors and help the user.
    // Composer install with incorrect PHP version.
    if (strpos($command, 'composer install') !== FALSE && strpos($output, 'your PHP version') !== FALSE && strpos($output, 'does not satisfy that requirement') !== FALSE) {
      drush_log(dt('Composer indicated that it could not install packages because the site codebase requires a higher version of PHP than what is running on the server.'), 'p_log');
      drush_log(dt('You can either upgrade your version of PHP on this server or set the maximum PHP version in your composer.json file. See !link for instructions for how to modify your composer.json file, then make sure to run `composer update` on your codebase to pin your packages to a lower PHP version.', [
        '!link' => '<a href="https://getcomposer.org/doc/06-config.md#platform">https://getcomposer.org/doc/06-config.md#platform</a>',
      ]), 'p_log');
    }

  }
}

