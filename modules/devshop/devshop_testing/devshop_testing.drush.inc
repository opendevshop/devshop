<?php


/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for test tasks.
 */
function devshop_testing_post_hosting_test_task($task, $data) {

  // We don't really need this if we are saving to a file.
//  // Get test results
//  // @TODO: Handle Simpletest as well
//  if (isset($data['context']['behat_test_results'])) {
//    $results = $data['context']['behat_test_results'];
//  }
//
//  // Save test run results as a task arg
//  $task->task_args['test_results'] = $results;
//  $task->task_args['test_results_file'] = $data['context']['behat_test_results_file'];
//  node_save($task);
}

/**
 * @param $task
 * @param $data
 */
function devshop_testing_hosting_test_task_rollback($task, $data) {
  devshop_testing_post_hosting_test_task($task, $data);
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for devshop deploy tasks.
 *
 * Saves a deploy task if the environment is set to do so.
 */
function devshop_testing_post_hosting_devshop_deploy_task($task, $data) {
  if ($task->ref->environment->settings->deploy['test']) {
    hosting_add_task($task->ref->nid, 'test');
  }
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for IMPORT tasks
 *
 * Ensures that cloned sites that are on project platforms inherit the project.
 */
function devshop_testing_post_hosting_import_task($task, $data)
{
  // If a site is being imported within a project...
  if ($task->ref->type == 'site' && !empty($task->ref->project)) {
    // .. and is configured to run tests on deploy...
    if ($task->ref->environment->settings->deploy['test']) {
      // trigger a test run.
      hosting_add_task($task->ref->nid, 'test');
    }
  }
}
