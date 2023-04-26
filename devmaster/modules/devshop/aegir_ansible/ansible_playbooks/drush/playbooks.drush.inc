<?php

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
    'options' => array(
      'inventory' => 'Ansible inventory file path or comma separated host list.',
      'limit' => 'Limit selected hosts to an additional pattern.',
      'user' => 'Connect as this user.',
      'connection' => 'Connection type to use.',
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

  if ($ansible_user = drush_get_option('user', '')) {
    drush_log('Connecting as user ' . $ansible_user, 'status');
    $ansible->user($ansible_user);
    $command .= "-u {$ansible_user} ";
  }

  if ($ansible_limit = drush_get_option('limit', '')) {
    drush_log('Limiting playbook run to ' . $ansible_limit, 'status');
    $ansible->limit($ansible_limit);
    $command .= "-l {$ansible_limit} ";
  }

  if ($ansible_inventory = drush_get_option('inventory', '')) {
    $ansible->inventoryFile($ansible_inventory);
    $command .= "-i {$ansible_inventory} ";
  }

  if ($ansible_connection = drush_get_option('connection', '')) {
    $ansible->connection($ansible_connection);
    $command .= "-c {$ansible_connection} ";
  }

  $is_devshop = drush_get_option('is-devshop', FALSE);

  drush_log("Running '$command'", $is_devshop? 'p_command': 'status');

  $exit = $ansible->execute(function ($type, $buffer) {
    if (drush_get_option('is-devshop', FALSE)) {
      drush_log($buffer, 'p_command_log');
    }
    else {
      print $buffer;
    }
  });

  if ($exit != 0) {
    drush_log(dt('Ansible playbook failed to complete.'), 'p_error');
    drush_set_error('DRUSH_ERROR', 'Ansible command exited with non-zero code.');
  }
  else {
    drush_log(dt('Ansible playbook complete!'), 'p_ok');
  }
}

/**
 * Implements drush_HOOK_pre_COMMAND()
 */
function drush_playbooks_pre_hosting_task() {
  $task = &drush_get_context('HOSTING_TASK');
  if ($task->task_type == 'playbook') {
    $task->args[] = $task->task_args['playbook'];
    $task->options['inventory'] = $task->task_args['inventory'];
    $task->options['limit'] = $task->task_args['limit'];
    $task->options['user'] = $task->task_args['user'];
    $task->options['connection'] = $task->task_args['connection'];
  }
}
