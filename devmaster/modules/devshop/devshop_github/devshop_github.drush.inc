<?php

// Include vendors
require_once 'vendor/autoload.php';

/**
 * Implements hook_hosting_task_update_status()
 */
function devshop_github_hosting_task_update_status($task, $status) {

  drush_log('[DEVSHOP GITHUB] Responding to hook update status...');

  // On initial pull request, the "clone" task is what runs.
  if ($task->task_type == 'clone') {
    $platform = node_load($task->task_args['target_platform']);
    $environment = $platform->environment;
    $project = $platform->project;
  }
  // Subsequent tasks are "deploy" and "verify"
  elseif ($task->task_type == 'verify' || $task->task_type == 'devshop-deploy' || $task->task_type == 'test') {
    $environment = $task->ref->environment;
    $project = $task->ref->project;
  }
  else {
    return;
  }

  // If a pull request object is available...
  if (isset($environment->github_pull_request->pull_request_object->deployment)) {

    // Create a deployment status
    $owner = $project->github_owner;
    $repo = $project->github_repo;
    $deployment_id = $environment->github_pull_request->pull_request_object->deployment->id;

    try {
      $token = variable_get('devshop_github_token', '');
      $client = new \Github\Client();
      $client->authenticate($token, Github\Client::AUTH_HTTP_TOKEN);

      $params = new stdClass();
      if ($status == HOSTING_TASK_SUCCESS || $status == HOSTING_TASK_WARNING) {
        $params->state = $state = 'success';
      }
      else {
        $params->state = $state = 'failure';
      }

      // If task is a test run, only submit a commit status for devshop/tests context.
      if ($task->task_type == 'test') {
        $sha = $environment->github_pull_request->pull_request_object->head->sha;

        $params = new stdClass();
        $params->state = $state;
        $params->target_url = url("devshop_tests/{$task->nid}/{$task->vid}", array('absolute' => TRUE));;

        if ($status == HOSTING_TASK_WARNING) {
          $params->description = t('DevShop: Tests passed with warnings');
        }
        else {
          $params->description = t('DevShop: Tests !status!', array('!status' => $state));
        }
        $params->context = 'devshop/tests';

        $status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
        drush_log('Commit status created for devshop/tests!', 'success'); 
      } 
      // Otherwise we create a deployment and a commit status.
      else {

      
      $params->target_url = $task->ref->environment->url;
      $params->description = t('Pull Request environment is ready!');
      $post_url = "/repos/$owner/$repo/deployments/{$deployment_id}/statuses";

      drush_log('Attempting to create github deployment status: ' . $post_url, 'success');

      $deployment_status = $client->getHttpClient()->post($post_url, json_encode($params));
      drush_log('Deployment status created!', 'success');


      // Update Status API

      // Create a status
      $sha = $environment->github_pull_request->pull_request_object->head->sha;

      $params = new stdClass();
      $params->state = $state;
      $params->target_url = url("node/{$task->nid}", array('absolute' => TRUE));;

      if ($status == HOSTING_TASK_WARNING) {
        $params->description = t('DevShop: Deploy success with warnings');
      }
      else {
        $params->description = t('DevShop: Deploy !status', array('!status' => $state));
      }
      $params->context = 'devshop/deploy';

      $status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
      drush_log('Commit status created!', 'success');

      }
    } catch (Github\Exception\RuntimeException $e) {
      drush_log('GitHub API Error: ' . $e->getMessage(), 'error');
    }
  }
}
