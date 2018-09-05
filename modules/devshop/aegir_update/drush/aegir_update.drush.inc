<?php

/**
 * @file
 * Provision/Drush hooks for the provision_tasks_extra module.
 *
 * These are the hooks that will be executed by the drush_invoke function.
 */

/**
 * Implementation of hook_drush_command().
 */
function aegir_update_drush_command() {
  $items['provision-update_drupal'] = array(
    'description' => 'Updates drupal and contributed packages.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  return $items;
}

/**
 * Implements the provision-update command.
 */
function drush_aegir_update_provision_update_drupal() {
  drush_errors_on();

  if (drush_drupal_major_version(d()->root) == 8) {
//    drush_log('Drush command provision-update_drupal is not tet compatible with Drupal 8. Run the command `composer update drupal/core webflo/drupal-core-require-dev symfony/* --with-dependencies');

    $path = d()->platform->repo_path? d()->platform->repo_path: d()->platform->root;
    if (file_exists($path . '/composer.json')) {
      provision_process('composer update drupal/core webflo/drupal-core-require-dev symfony/* --with-dependencies', $path);
    }
    else {
      drush_set_error('PROVISION_ERROR', dt('Drupal 8 was detected, but a composer.json file was not found in %path. Updating Drupal 8 automatically requires using a composer-based platform.  See https://github.com/drupal-composer/drupal-project for the starting template.', [
        '%path' => $path,
      ]));
    }
  }
  elseif (drush_drupal_major_version(d()->root) <= 7) {
    provision_backend_invoke(d()->name, 'pm-update');
  }
  drush_log(dt('Drush pm-update task completed'));
}

/**
 * Map values of site node into command line arguments.
 */
function drush_aegir_update_pre_hosting_task($task) {
  $task = &drush_get_context('HOSTING_TASK');
  if ($task->task_type == 'update_drupal' && !empty($task->task_args['commit'])) {
    // Pass the argument provision_git expects.
    $task->options['commit'] = $task->task_args['commit'];
  }
}
