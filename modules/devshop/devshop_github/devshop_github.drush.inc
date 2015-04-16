<?php

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
 */
function drush_devshop_github_pre_hosting_task() {

  $task =& drush_get_context('HOSTING_TASK');
  if ($task->ref->type == 'site' && $task->task_type == 'test') {

    // Include vendors
    require_once 'vendor/autoload.php';

    drush_log('===========================================', 'ok');
    drush_log('Notifying github...', 'ok');


    try {
      $token = variable_get('devshop_github_token', '');
      $client = new \Github\Client();
      $client->authenticate($token, Github\Client::AUTH_HTTP_TOKEN);

      // Create a status
      $sha = $task->ref->environment->github_pull_request->pull_request_object->head->sha;

      $params = new stdClass();
      $params->state = 'pending';
      $params->target_url = url("devshop_tests/{$task->nid}/{$task->vid}", array('absolute' => TRUE));;
      $params->description = 'Test run has been queued.';
      $params->context = 'devshop/test';

      $owner = $task->ref->project->github_owner;
      $repo = $task->ref->project->github_repo;

      $status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));

      drush_log('Status posted! ', 'ok');
    } catch (Github\Exception\RuntimeException $e) {
      drush_log('GitHub API Error: ' . $e->getMessage(), 'error');
      drush_log(l(t('Configure GitHub API'), 'admin/hosting/devshop/github'), 'error');
    } catch (Github\Exception\ValidationFailedException $e) {
      drush_log('GitHub API Error: ' . $e->getMessage(), 'error');
    }


  }
}