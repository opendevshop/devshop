<?php

/**
 * Implements hook_hosting_site_context_options()
 *
 * Runs on verify task. Passes data to the drush alias.
 * Save environment name, project name, and git ref to site aliases.
 */
function devshop_cloud_hosting_server_context_options(&$task) {
  if (isset($task->ref->services['provider'])) {
    $task->context_options['provider'] = $task->ref->services['provider']->type;
    $task->context_options['provider_options'] = $task->ref->services['provider']->provider_options;
    $task->context_options['provider_server_identifier'] = $task->ref->services['provider']->provider_server_identifier;
  }
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for delete tasks.
 */
function devshop_cloud_post_hosting_verify_task($task, $data) {

  // When a project is deleted...
  if ($task->ref->type == 'server') {
    drush_log('[DEVSHOP] Server Verified... Saving info to front-end.', 'ok');

    drush_log('[DEVSHOP] devshop_cloud_post_hosting_verify_task ' . print_r($data['context'], 1), 'ok');

    $task->ref->services['provider']->provider_data = $data['context']['provider_data'];
    $task->ref->services['provider']->provider_server_identifier = $data['context']['provider_server_identifier'];

    if (empty($task->ref->ip_addresses)){
      $task->ref->new_ip_addresses = $data['context']['ip_addresses'];
    }
    else {
      $task->ref->ip_addresses = $data['context']['ip_addresses'];
    }

    $task->ref->no_verify = TRUE;

    drush_log('[DEVSHOP] saving node... ' . print_r($task->ref, 1), 'ok');
    node_save($task->ref);
  }
}
