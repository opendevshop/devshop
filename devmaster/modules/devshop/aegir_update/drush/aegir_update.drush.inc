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

    $path = drush_get_option('git_root');
    $packages = drush_get_option('packages');
    if (file_exists($path . '/composer.json')) {
      provision_process(drush_get_option('composer_update_command', "composer update --ansi --no-progress") . " " . $packages, $path);
    }
    elseif (drush_drupal_major_version(d()->root) <= 7) {
      provision_backend_invoke(d()->name, 'pm-update');
    }
    else {
      drush_set_error('PROVISION_ERROR', dt('Unable to find composer.json or Drupal at %path', array('%path' => $path)));
    }
}

/**
 * Map values of site node into command line arguments.
 */
function drush_aegir_update_pre_hosting_task($task) {
  $task = &drush_get_context('HOSTING_TASK');
  if ($task->task_type == 'update_drupal') {
    $task->options['packages'] = $task->task_args['packages'];
    
    if (!empty($task->task_args['commit'])) {
      // Pass the argument provision_git expects.
      $task->options['commit'] = $task->task_args['commit'];
    }
  }
}
