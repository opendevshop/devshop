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
  // Only trigger deployments for the specified tasks.
  // Every type of task listed here will have a GitHub "deployment" object
  // created for it, and that deployment will have it's status updated.
  if (empty($task->ref->environment) || $task->ref->environment->git_provider != 'bitbucket' || !in_array($task->task_type, variable_get('devshop_bitbucket_deployment_tasks', array(
          'deploy',
          'install',
          'test'
      )))) {
      return;
  }

    $task->ref = node_load($task->rid);
    $environment = $task->ref->environment;

    if (!file_exists($task->ref->git_root)) {
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
        '@action' => $task->task_type == 'install'? t('Install'):
          ($task->task_type == 'test'? t('Test'):
          (!empty($task->task_args['test'])? t('Test'): t('Deploy'))),
        '@env' => $task->ref->hosting_environment,
      ];

      if ($status == HOSTING_TASK_PROCESSING) {
        $params->state = $state = 'INPROGRESS';
        $params->name = t('@action started: @env', $t);
      }
      elseif ($status == HOSTING_TASK_SUCCESS || $status == HOSTING_TASK_WARNING) {

        // If there is a parent task, the build is not done yet.
        if (!empty($task->task_args['parent_task'])) {
          $params->state = $state = 'INPROGRESS';
          $t['@action'] .= ' '. t('Additional tasks pending.');
        }
        else {
          $params->state = $state = 'SUCCESSFUL';
        }

        if ($status == HOSTING_TASK_SUCCESS) {
          $params->name = t('@action Success: @env', $t);
        }
        else {
          $params->name = t('@action Success (with warnings): @env', $t);
        }
      }
      else {
        $params->state = $state = 'FAILED';
        $params->name = t('@action Failed: @env', $t);
      }

      $params->key = "deploy.{$task->ref->hosting_group}.{$task->ref->hosting_environment}";
      $params->url = url("node/$task->nid", ['absolute' => true]);
      $params->description = $task->ref->url;

      $build_status = $client->repositories()->workspaces($environment->bitbucket_owner)->commit($environment->bitbucket_repo)->statuses($sha)->build()->create((array) $params);

      if (!empty($build_status)) {
        drush_log("Build status: " . print_r($build_status,1), 'debug');
        drush_log(t("Posted @state build status to BitBucket for commit @commit @url", [
          '@state' => $build_status['state'],
          '@commit' => $build_status['commit']['hash'],
          '@url' => $build_status['commit']['links']['html']['href'],
        ]), 'p_log');
      }

    } catch (\Exception $e) {
      return drush_set_error(DRUSH_APPLICATION_ERROR, 'bitbucket API Error: ' . $e->getMessage());
    }
}
