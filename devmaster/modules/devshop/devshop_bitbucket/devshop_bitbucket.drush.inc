<?php
//
//// Include vendors
//require_once 'vendor/autoload.php';
//
///**
// * Implements hook_hosting_task_update_status().
// */
//function devshop_bitbucket_hosting_task_update_status($task, $status) {
//
//  drush_log('[DEVSHOP bitbucket] Responding to hook update status...');
//
//  // On initial pull request, the "clone" task is what runs.
//  if ($task->task_type == 'clone') {
//    $platform = node_load($task->task_args['target_platform']);
//    $environment = $platform->environment;
//    $project = $platform->project;
//  }
//  // Subsequent tasks are "deploy" and "verify"
//  elseif (($task->task_type == 'verify' && $task->ref->type == 'site') || $task->task_type == 'devshop-deploy' || $task->task_type == 'test') {
//    $environment = $task->ref->environment;
//    $project = $task->ref->project;
//  }
//  else {
//    return;
//  }
//
//  // If a pull request object is available...
//  if (isset($environment->bitbucket_pull_request->pull_request_object->deployment)) {
//
//    // Create a deployment status
//    $owner = $project->bitbucket_owner;
//    $repo = $project->bitbucket_repo;
//    $deployment_id = $environment->bitbucket_pull_request->pull_request_object->deployment->id;
//
//    try {
//      $token = variable_get('devshop_bitbucket_token', '');
//      $client = new \bitbucket\Client();
//      $client->authenticate($token, bitbucket\Client::AUTH_HTTP_TOKEN);
//
//      $params = new stdClass();
//      if ($status == HOSTING_TASK_SUCCESS || $status == HOSTING_TASK_WARNING) {
//        $params->state = $state = 'success';
//      }
//      else {
//        $params->state = $state = 'failure';
//      }
//
//      // If task is a test run, only submit a commit status for devshop/tests context.
//      if ($task->task_type == 'test') {
//        $sha = $environment->bitbucket_pull_request->pull_request_object->head->sha;
//
//        $params = new stdClass();
//        $params->state = $state;
//        $params->target_url = url("node/{$task->nid}/revisions/{$task->vid}/view",array('absolute' => TRUE));
//
//        if ($status == HOSTING_TASK_WARNING) {
//          $params->description = t('DevShop: Tests passed with warnings');
//        }
//        else {
//          $params->description = t('DevShop: Tests !status!', array('!status' => $state));
//        }
//        $params->context = 'devshop/tests';
//
//        $status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
//        drush_log('Commit status created for devshop/tests!', 'success');
//      }
//      // Otherwise we create a deployment and a commit status.
//      else {
//
//      $params->target_url = $environment->url;
//      $params->description = t('Visit !url', array('!url' => $task->ref->environment->url));
//      $post_url = "/repos/$owner/$repo/deployments/{$deployment_id}/statuses";
//
//      drush_log('Attempting to create bitbucket deployment status: ' . $post_url, 'success');
//
//      $deployment_status = $client->getHttpClient()->post($post_url, json_encode($params));
//      drush_log('Deployment status created!', 'success');
//
//
//      // Update Status API
//
//      // Create a status
//      $sha = $environment->bitbucket_pull_request->pull_request_object->head->sha;
//
//      $params = new stdClass();
//      $params->state = $state;
//      $params->target_url = url("node/{$task->nid}", array('absolute' => TRUE));;
//
//      if ($status == HOSTING_TASK_WARNING) {
//        $params->description = t('DevShop: Deploy success with warnings. [!url]', array(
//          '!url' => $environment->url,
//        ));
//      }
//      else {
//        $params->description = t('DevShop: Deploy !status [!url]', array(
//          '!status' => $state,
//          '!url' => $environment->url,
//        ));
//      }
//      $params->context = 'devshop/deploy';
//
//      $deployment_status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
//      drush_log('Commit status created!', 'success');
//
//        // If deploy task fails, tests won't run.
//        if ($environment->settings->deploy['test'] && $status == HOSTING_TASK_ERROR) {
//
//          $params = new stdClass();
//          $params->state = $state;
//            $params->description = t('DevShop: Tests not run due to Deploy Fail');
//          $params->context = 'devshop/tests';
//
//          $deployment_status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
//          drush_log('Commit status created for devshop/tests', 'success');
//        }
//      }
//    } catch (bitbucket\Exception\RuntimeException $e) {
//      drush_log('bitbucket API Error: ' . $e->getMessage(), 'error');
//    }
//  }
//}
