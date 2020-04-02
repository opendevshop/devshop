<?php

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 * for Verify tasks.
 *
 * Runs the "verify" dotHook
 */
function devshop_dothooks_post_hosting_verify_task($task, $data) {
  if ($task->ref->type == 'site') {
    devshop_dothooks_run_hook('verify', $task->ref, $task);
  }
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 * for DevShop Deploy tasks.
 *
 * Runs the "deploy" dotHook
 */
function devshop_dothooks_post_hosting_devshop_deploy_task($task, $data) {
  devshop_dothooks_run_hook('deploy', $task->ref, $task);
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 * for Run Tests tasks.
 *
 * Runs the "test" dotHook
 */
function devshop_dothooks_post_hosting_test_task($task, $data) {
  devshop_dothooks_run_hook('test', $task->ref, $task);
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 * for Sync tasks.
 *
 * Runs the "sync" dotHook
 */
function devshop_dothooks_post_hosting_sync_task($task, $data) {
  devshop_dothooks_run_hook('sync', $task->ref, $task);
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 * for Install tasks.
 *
 * Runs the "install" dotHook
 */
function devshop_dothooks_post_hosting_install_task($task, $data) {
  devshop_dothooks_run_hook('install', $task->ref, $task);
}
