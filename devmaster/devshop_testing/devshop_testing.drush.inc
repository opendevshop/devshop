<?php


/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for devshop deploy tasks.
 */
function devshop_testing_post_hosting_test_task($task, $data) {

  // Get test results
  // @TODO: Handle Simpletest as well
  if (isset($data['context']['behat_test_results'])) {
    $results = $data['context']['behat_test_results'];
  }

  // Save test run results as a task arg
  $task->task_args['test_results'] = $results;
  node_save($task);
}

/**
 * @param $task
 * @param $data
 */
function devshop_testing_hosting_test_task_rollback($task, $data) {
  devshop_testing_post_hosting_test_task($task, $data);
}