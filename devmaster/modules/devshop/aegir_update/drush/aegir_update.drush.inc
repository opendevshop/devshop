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
  provision_backend_invoke(d()->name, 'pm-update');
  drush_log(dt('Drush pm-update task completed'));
}