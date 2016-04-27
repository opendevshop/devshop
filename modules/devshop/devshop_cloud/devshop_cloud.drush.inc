<?php

/**
 * Implements hook_hosting_site_context_options()
 *
 * Runs on verify task. Passes data to the drush alias.
 * Save environment name, project name, and git ref to site aliases.
 */
//function devshop_cloud_hosting_server_context_options(&$task) {
//  if (isset($task->ref->services['provider'])) {
//    $task->context_options['provider'] = $task->ref->services['provider']->type;
//    $task->context_options['provider_options'] = $task->ref->services['provider']->provider_options;
//    $task->context_options['provider_server_identifier'] = $task->ref->services['provider']->provider_server_identifier;
//  }
//}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for verified tasks.
 */
//function devshop_cloud_post_hosting_verify_task($task, $data) {
////   When a project is verified...
//  if ($task->ref->type == 'server') {
//    if (empty($data['context']['provider_server_identifier'])) {
//      return drush_set_error('DEVSHOP_CLOUD_SERVER_ID_NOT_FOUND', 'The provider did not return a server identifier.');
//    }
//
//     Save Provider Data and ID to the service in the front-end.
//    if (db_query("UPDATE {hosting_devshop_cloud_provider} SET provider_data = '%s', provider_server_identifier = '%s' WHERE nid = %d", serialize($data['context']['provider_data']), $data['context']['provider_server_identifier'], $task->ref->nid)) {
//      drush_log(dt('[DEVSHOP] Saving server ID &id and data for &server', array(
//        '&id' => $data['context']['provider_server_identifier'],
//        '&server' => $task->ref->title,
//      )), 'ok');
//    }
//    else {
//      return drush_set_error('DEVSHOP_CLOUD_UNABLE_TO_SAVE', dt('Unable to save server ID and data to database.'));
//    }
//
//     Save IP addresses.
//    if (empty($task->ref->ip_addresses)){
//      $task->ref->new_ip_addresses = $data['context']['ip_addresses'];
//    }
//    else {
//      $task->ref->ip_addresses = $data['context']['ip_addresses'];
//    }
//  }
//}
