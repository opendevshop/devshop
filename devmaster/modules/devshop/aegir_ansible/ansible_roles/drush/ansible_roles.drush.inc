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
    require_once('Provision/Config/AnsibleConfig.php');
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

  // Only hostmaster gets ansible_roles_available.
  if (isset($task->ref->hosting_name) && $task->ref->hosting_name == 'hostmaster' && isset($task->ref->ansible_roles_available)) {
    $task->context_options['ansible_roles_available'] = $task->ref->ansible_roles_available;
  }
}

/**
 * Implements drush_hook_pre_hosting_task()
 */
function drush_ansible_roles_pre_hosting_task() {

  $task =& drush_get_context('HOSTING_TASK');

  if ($task->task_type == 'verify' && $task->ref->hosting_name == 'hostmaster' ) {

    if (empty($task->ref->ansible_role_file)) {
      drush_log("No Ansible Galaxy roles loaded from hostmaster node.", 'ok');
      return;
    }
    // First we must save a roles.yml file so we can install custom ones.
    // @see ansible_roles_node_load()
    $filename = "private://hostmaster-roles-{$task->ref->vid}.yml";
    $real_file_path = file_unmanaged_save_data($task->ref->ansible_role_file, $filename);

    if (file_exists($real_file_path)) {
      drush_log('Ansible roles file saved to ' . $real_file_path, 'p_log');
      drush_log($task->ref->ansible_role_file, 'p_ok');
    }
    else {
      return drush_set_error('DRUSH_ERROR', dt('Unable to generate roles.yml file.'));
    }

    $task->options['roles_file_path'] = drupal_realpath($real_file_path);
  }
}

/**
 * Implements drush_hook_pre_COMMAND()
 *
 * This kicks off for provision-verify
 */
function drush_ansible_roles_pre_provision_verify () {
  if (drush_get_option('roles_file_path', NULL)) {

    // Write ansible.cfg file.
    $ansible_cfg = new Provision_Config_AnsibleConfig(d());
    $ansible_cfg->write();
    drush_log('Ansible.cfg written to ' . $ansible_cfg->filename(), 'p_log');

    $ansible = new Provision_Service_Ansible(d());
    $ansible->runRolesInstall();
  }
}
