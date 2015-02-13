<?php

/**
 * Implements hook_hosting_site_context_options()
 *
 * Runs on verify task. Passes data to the drush alias.
 * Save environment name, project name, and git ref to site aliases.
 */
function devshop_cloud_hosting_server_context_options(&$task) {
  if (isset($task->ref->provider)) {
    $task->context_options['provider'] = $task->ref->provider;
    $task->context_options['provider_options'] = $task->ref->provider_options;
  }
}


/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for delete tasks.
 */
function devshop_cloud_post_hosting_verify_task($task, $data) {

  // When a project is deleted...
  if ($task->ref->type == 'server') {
    drush_log('[DEVSHOP] Server Verified... Saving info to front-end.', 'ok');
  }
}