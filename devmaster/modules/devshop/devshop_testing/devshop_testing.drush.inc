<?php

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for devshop deploy tasks.
 *
 * Saves a deploy task if the environment is set to do so.
 */
function devshop_testing_post_hosting_devshop_deploy_task($task, $data) {
  if ($task->ref->environment->settings->deploy['test']) {
    $node = hosting_add_task($task->ref->nid, 'test');
    drush_log(l(t('A Test task has been queued.'), "node/{$node->nid}"), 'p_log');
  }
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for IMPORT tasks
 *
 * Ensures that cloned sites trigger a test run if their environment on first deploy.
 * This is used when a Pull Request environment is created.
 */
function devshop_testing_post_hosting_import_task($task, $data)
{
  // Only work with site import.
  if ($task->ref->type != 'site') {
    return;
  }

  // Load environment info from platform, because site node doesn't have info yet.
  $platform = node_load($task->ref->platform);

  // If environment settings say to run tests on deploy...
  if (isset($platform->environment->settings->deploy) && $platform->environment->settings->deploy['test']) {
    drush_log('[DEVSHOP] Triggering test run...', 'ok');
    hosting_add_task($task->ref->nid, 'test');
  }
  else {
    drush_log('[DEVSHOP] Skipping test run...', 'ok');
  }
}
