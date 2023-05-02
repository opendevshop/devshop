<?php

/**
 * Pass github token variable to the back end.
 *
 * @param $task
 */
function drush_devshop_bitbucket_pre_hosting_task()
{
  drush_set_option('devshop_bitbucket_token', variable_get('devshop_bitbucket_token', null));
}

/**
 * Implements hook_hosting_task_update_status().
 */
function devshop_bitbucket_hosting_task_update_status($task, $status) {

  if ($task->task_type == 'install' || $task->task_type == 'deploy' || $task->task_type == 'test') {
    $task->ref = node_load($task->rid);
    $environment = $task->ref->environment;

    if (empty($environment->bitbucket_owner)) {
      return;
    }

    // Post a build status.
    try {
      
      $client = devshop_bitbucket_client();
      $params = new stdClass();

      $command = "git ls-remote origin {$task->ref->git_reference}";
      list($sha, $null) = explode('	', provision_process($command, $task->ref->git_root));

      drush_log(dt('Found remote SHA of %sha for branch %ref with command: %command', [
        '%sha' => $sha,
        '%ref' => $task->ref->git_reference,
        '%command' => $command,
      ]), 'debug');
      
      $t = [
        '@env' => "{$task->ref->hosting_group}.{$task->ref->hosting_environment}"
      ];
      
      if ($status == HOSTING_TASK_PROCESSING) {
        $params->state = $state = 'INPROGRESS';
        $params->name = t('DevShop Deploy to @env: In Progress', $t);
      }
      elseif ($status == HOSTING_TASK_SUCCESS || $status == HOSTING_TASK_WARNING) {
        $params->state = $state = 'SUCCESSFUL';
        if ($status == HOSTING_TASK_SUCCESS) {
          $params->name = t('DevShop Deploy to @env: Success', $t);
        }
        else {
          $params->name = t('DevShop Deploy to @env: Success (with warnings)', $t);
        }
      }
      else {
        $params->state = $state = 'FAILED';
        $params->name = t('DevShop Deploy to @env: Failed', $t);
      }

      $params->key = "deploy.{$task->ref->hosting_group}.{$task->ref->hosting_environment}";
      $params->url = $task->ref->url;
      $params->description = url("node/$task->nid", ['absolute' => true]);
      
      $build_status = $client->repositories()->workspaces($environment->bitbucket_owner)->commit($environment->bitbucket_repo)->statuses($sha)->build()->create((array) $params);
      
      if (!empty($build_status)) {
        drush_log("Build status: " . print_r($build_status,1), 'debug');
      }
      
    } catch (\Exception $e) {
      return drush_set_error(DRUSH_APPLICATION_ERROR, 'bitbucket API Error: ' . $e->getMessage());
    }
  }
}
