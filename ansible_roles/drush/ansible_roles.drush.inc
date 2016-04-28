<?php

/**
 * @file
 *   An ansible_roles of the provision service API.
 *
 *  Declares a new service type and a basic implementation of it.
 *  It matches the same service definition in the hosting front end.
 */

/**
 * Implements hook_drush_init().
 */
function ansible_roles_drush_init()
{
  if (class_exists('Provision_Service')) {
    require_once('Provision/Service/server/ansible_roles_available.php');
  }
}

/**
 * Expose a service to servers
 *
 * @return
 *   An array with the service type the key, and the default implementation the value.
 */
function ansible_roles_provision_services() {
  ansible_roles_drush_init();
  return array('ansible_roles_available' => NULL);
}

/**
 * Implements hook_hosting_server_context_options().
 */
function ansible_roles_hosting_server_context_options(&$task) {

  // Only server_master gets ansible_roles_available.
  if (isset($task->ref->hosting_name) && $task->ref->hosting_name == 'server_master' && isset($task->ref->ansible_roles_available)) {
    $task->context_options['ansible_roles_available'] = $task->ref->ansible_roles_available;
  }
}
