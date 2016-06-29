<?php

require_once(__DIR__ . '/../../vendor/autoload.php');

use Asm\Ansible\Ansible;

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
 * Drush validate hook for provision-playbook command
 * @param null $playbook_file
 * @return mixed
 */
function drush_playbooks_provision_playbook_validate($playbook_file = NULL) {
  if (empty($playbook_file)) {
    return drush_set_error('DRUSH_COMMAND_ERROR', dt('No playbook file specified.'));
  }
}

/**
 * Implements the provision-flush_cache command.
 */
function drush_playbooks_provision_playbook($playbook_file) {
  drush_errors_on();

  // Prepare the Ansible object.
  $ansibleObject = new Ansible(
    getcwd(),
    '/usr/bin/ansible-playbook',
    '/usr/bin/ansible-galaxy'
  );

  // Set Timeout to 30 minutes
  $ansibleObject->setTimeout(60 * 30);

  $ansible = $ansibleObject->playbook();

  $ansible->play($playbook_file);
  $command = "ansible-playbook {$playbook_file} ";

  if ($ansible_user = drush_get_option('ansible_user', 'root')) {
    drush_log('Connecting as user ' . $ansible_user, 'status');
    $ansible->user($ansible_user);
    $command .= "-u {$ansible_user} ";
  }

  if ($ansible_limit = drush_get_option('ansible_limit', '')) {
    drush_log('Limiting playbook run to ' . $ansible_limit, 'status');
    $ansible->limit($ansible_limit);
    $command .= "-l {$ansible_limit} ";
  }

  if ($ansible_inventory = drush_get_option('ansible_inventory', '')) {
    $ansible->inventoryFile($ansible_inventory);
    $command .= "-i {$ansible_inventory} ";
  }

  $is_devshop = drush_get_option('is-devshop', FALSE);

  drush_log("Running '$command'", $is_devshop? 'devshop_command': 'status');

  $exit = $ansible->execute(function ($type, $buffer) {
    if (drush_get_option('is-devshop', FALSE)) {
      drush_log($buffer, 'devshop_info');
    }
    else {
      print $buffer;
    }
  });

  if ($exit != 0) {
    drush_log(dt('Ansible playbook failed to complete.'), 'devshop_error');
    drush_set_error('DRUSH_ERROR', 'Ansible command exited with non-zero code.');
  }
  else {
    drush_log(dt('Ansible playbook complete!'), 'devshop_ok');
  }
  drush_log('ANSIBLE PLAYBOOK COMING SOON', 'devshop_ok');

}