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
  require_once('Provision/Service/db/ansible_mysql.php');
  require_once('Provision/Service/http/ansible_apache.php');
}

/**
 * Implements drush_hook_pre_COMMAND()
 *
 * This kicks off for provision-verify
 */
function drush_ansible_roles_pre_provision_verify () {
  if (d()->type == 'server') {
    d()->service('http')->runPlaybook();
  }
}


/**
 * Implements drush_HOOK_pre_COMMAND()
 *
 * This runs for each task during the command
 *    drush @hostmaster hosting-tasks
 *
 * NOTE: This ONLY runs when being called from a hostmaster task.
 * This hook should ONLY be used to pass options from a hostmaster task form to
 * the $task object, or if you don't need this functionality from the command
 * line.
 *
 * Server Verify:
 *
 * - Load all roles from the aegir services
 * - Generate a playbook.yml file.
 * - Pass the path of the playbook as a drush option to Provision_Service_http_ansible_apache
 *
 */
function drush_ansible_roles_pre_hosting_task()
{
  $task =& drush_get_context('HOSTING_TASK');
  if ($task->ref->type == 'server' && $task->task_type == 'verify') {


    // Load all roles to generate a playbook.
    $roles = array();
    foreach ($task->ref->services as $service) {
      if (method_exists($service, 'roles')) {
        $roles = array_merge($roles, $service->roles());
      }
    }

    // Prepare an object to represent our playbook.
    $playbook = array();
    $playbook[] = array(
        'hosts' =>$task->ref->title,
        'roles' => $roles,
    );

    // Save the object to YML
    $yml = new \Symfony\Component\Yaml\Dumper();
    $output = "---\n" . $yml->dump($playbook, 10, 2);

    // Save the playbook to a file.
    $playbook_path = 'private://playbooks';
    file_prepare_directory($playbook_path, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);

    $filename = "private://playbooks/playbook.task-{$task->nid}.yml";
    $real_file_path = file_unmanaged_save_data($output, $filename);

    if (file_exists($real_file_path)) {
      drush_log('Ansible playbook file saved to ' . $real_file_path, 'devshop_log');
    }
    else {
      return drush_set_error('DRUSH_ERROR', dt('Unable to generate playbook.yml file. Run `drush @hostmaster provision-verify` to try and fix file permissions.'));
    }

    $task->options['playbook'] = drupal_realpath($real_file_path);
  }
}