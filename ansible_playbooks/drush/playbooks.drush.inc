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
function playbooks_drush_command() {
  $items['provision-playbook'] = array(
    'description' => 'Runs an ansible playbook.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'playbook_file' => 'Path to a playbook.yml file.',
    ),
  );
  return $items;
}

/**
 * Implements the provision-flush_cache command.
 */
function drush_playbooks_provision_playbook() {
  drush_errors_on();

  drush_log('ANSIBLE PLAYBOOK COMING SOON', 'devshop_ok');

}