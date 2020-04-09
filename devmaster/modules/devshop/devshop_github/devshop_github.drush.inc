<?php

/**
 * Implements hook_drush_command().
 * Provides the "platform-delete" command.  This command triggers a delete task on a platform specified by it's path.
 *
 * This allows taking the final step in an upgrade: deleting the old devmaster.
 */
function devshop_github_drush_command()
{
  $items['github-deploy'] = array(
    'description' => 'Create a deployment and deployment status for the chosen environment.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
  );
  return $items;
}

/**
 *
 */
function drush_devshop_github_github_deploy($alias, $state, $description = NULL) {
  drush_log("Git Reference: " . d($alias)->platform->git_ref, 'ok');

  $project_name = d($alias)->project;
  $environment_name = d($alias)->environment;
  $project_node = devshop_projects_load_by_name($project_name);

  $environment = $project_node->project->environments[$environment_name];
  $environment->project = $project_node->project;

//  DevShopGitHubApi::createDeployment($environment, $state, $description);

}
/**
 * Pass github token variable to the back end.
 *
 * @param $task
 */
function drush_devshop_github_pre_hosting_task()
{
  $task =& drush_get_context('HOSTING_TASK');

  // Pass github_deployment_id from task node to drush options.
  if ($task->task_type == 'devshop-deploy' && !empty($task->task_args['github_deployment_id'])) {
    $task->options['github_deployment_id'] = $task->task_args['github_deployment_id'];
  }
  // @TODO If task is install or devshop-deploy, erase the deployment status from the PR so a new one is created.
  // @TODO: We should erase and create a "pending" status when the task nodes are created.

  drush_set_option('devshop_github_token', variable_get('devshop_github_token', null));
}

/**
 * Implements hook_hosting_task_update_status().
 * This hook fires multiple times throughout the life of a hosting task.
 *
 * @param $task
 *   The task node being updated. @see hosting_task.module
 *   $task->task_status is the LAST status of the task.
 *   See $status for the state you want to trigger from.
 *
 * @param $status
 *    The "task_status" value the node is about to be updated with. This is the
 *    state you should check for.
 *
 *    $status can be:
 *      HOSTING_TASK_QUEUED;
 *      HOSTING_TASK_PROCESSING;
 *      HOSTING_TASK_ERROR;
 *      HOSTING_TASK_SUCCESS;
 *      HOSTING_TASK_WARNING;
 */
function devshop_github_hosting_task_update_status($task, $status) {

  // Only trigger deployments for the specified tasks.
  // Every type of task listed here will have a GitHub "deployment" object
  // created for it, and that deployment will have it's status updated.
  if (!in_array($task->task_type, variable_get('devshop_github_deployment_tasks', array(
    'devshop-deploy',
    'install',
    'test'
  )))) {
    return;
  }

  // @TODO: If this is a test task, should we just update the deployment of the last "devshop-deploy" task to "error"?
  // Or, do we let it ride with a 1-to-1 devshop task = github deployment relationship?
  // It would not be hard to run a DevShopGithubAPI:deploy() here if ths were a test (or other kind of task).

  // Reload the ref no matter what: we want fresh data.
  $task_reloaded = node_load($task->nid, NULL, TRUE);
  $task->ref = node_load($task->rid, NULL, TRUE);

  if (!isset($task->ref->environment)) {
    return;
  }

  $environment = $task->ref->environment;
  if ($status == HOSTING_TASK_WARNING) {
    $description = t('DevShop: !type success with warnings. [!url]', array(
      '!url' => $environment->url,
      '!type' => $task->task_type,
    ));
  }
  else {
    $description = t('DevShop: !type !status [!url]', array(
      '!status' => $status,
      '!url' => $environment->url,
      '!type' => $task->task_type,
    ));
  }

  // @TODO: Local SHA is only accurate after the git pull in "devshop-deploy".
  // @TODO: Might be able to decide based on $status if we grab the SHA or branch.
//  $sha = devshop_environment_get_git_sha($environment->repo_path);
  $sha = NULL;

  // Update the log URL because we know the Task NID now.
  $task_url = url("node/$task->nid", array(
    'absolute' => TRUE,
  ));

  // No Deployment ID. Create deployment.
  if (empty($task_reloaded->github_deployment->id)) {
    drush_log(dt('GitHub Deployment was not found for Task !url', array(
      '!url' => url("node/{$task_reloaded->nid}", array(
        'absolute' => TRUE,
      )),
    )), 'p_log');
  }
  else {
    drush_log(dt('GitHub Deployment !id was found for Task !url', array(
      '!id' => $task_reloaded->github_deployment->id,
      '!url' => url("node/{$task_reloaded->nid}", array(
        'absolute' => TRUE,
      )),
    )), 'p_log');
  }

  // @TODO: Change this to a simple task status/github state hash.
  if ($status == HOSTING_TASK_SUCCESS || $status == HOSTING_TASK_WARNING) {
    $state = 'success';
  }
  // @TODO: Enable preview queued and in_progress with:
  // media type in the Accept header:
  // application/vnd.github.flash-preview+json
  elseif ($status == HOSTING_TASK_QUEUED) {
    $state = 'queued';
    $state = 'pending';
  }
  elseif ($status == HOSTING_TASK_PROCESSING) {
    $state = 'in_progress';
    $state = 'pending';
  }
  else {
    $state = 'failure';
  }

//  // The method ::deploy() should return the existing deployment object for the $task_reloaded.
//  $deployment_object = DevShopGitHubApi::deploy($environment, $state, $task_reloaded, $description, $sha, $task_url);
//
//  if (!$deployment_object){
//    drush_log("@TODO: Unable to set new deployment status: $deployment_object Empty: [$task->nid] $state", 'warning');
//  }
//
//  // React only on certain task types and if project and environment exist.
//  $types = array('install', 'test', 'devshop-deploy');
//  if (empty($task->ref)) {
//    $task->ref = node_load($task->rid);
//  }
//  if (in_array($task->task_type, $types) && !empty($task->ref->project) && !empty($task->ref->environment)) {
//    $project = $task->ref->project;
//    $environment = $task->ref->environment;
//  }
//  else {
//    return;
//  }
//
//  // If a pull request object and Deployment is available...
////  if (isset($environment->github_pull_request->pull_request_object->deployment)) {
//
//    // If project is configured to reinstall every time, only react on "install" tasks. Otherwise, we get two github deployments because both a "deploy" (git pull) and a "install" task are run on each git push.
//// @TODO: Commenting for now because we mmight not need it, and we want it to work with non PR environments.
////    if ($project->settings->github['pull_request_reinstall'] && $task->task_type == 'devshop-deploy') {
////      return;
////    }
//
//    // Create a deployment status
////    $owner = $project->github_owner;
////    $repo = $project->github_repo;
////    $deployment_id = $environment->github_pull_request->pull_request_object->deployment->id;
////
////    try {
////      $token = variable_get('devshop_github_token', '');
////      $client = new \Github\Client();
////      $client->authenticate($token, Github\Client::AUTH_HTTP_TOKEN);
////
////      $params = new stdClass();
//      if ($status == HOSTING_TASK_SUCCESS || $status == HOSTING_TASK_WARNING) {
//        $state = 'success';
//      }
//      // @TODO: Enable preview queued and in_progress with:
//      // media type in the Accept header:
//      // application/vnd.github.flash-preview+json
//      elseif ($status == HOSTING_TASK_QUEUED) {
//        $state = 'queued';
//        $state = 'pending';
//      }
//      elseif ($status == HOSTING_TASK_PROCESSING) {
//        $state = 'in_progress';
//        $state = 'pending';
//      }
//      else {
//        $state = 'failure';
//      }
//
//      // If task is a test run, only submit a commit status for devshop/tests context.
//      // @TODO: Move to devshop_tests module
////      if ($task->task_type == 'test') {
////        $sha = $environment->github_pull_request->pull_request_object->head->sha;
////
////        $params = new stdClass();
////        $params->state = $state;
////        $params->target_url = url("node/{$task->nid}/view", array('absolute' => TRUE));
////
////        if ($status == HOSTING_TASK_WARNING) {
////          $params->description = t('DevShop: Tests passed with warnings');
////        }
////        else {
////          $params->description = t('DevShop: Tests !status!', array('!status' => $state));
////        }
////        $params->context = "devshop/{$project->name}/tests";
////
////        $status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
////        drush_log('Commit status created for devshop/tests!', 'success');
////      }
////      // Otherwise we create a deployment and a commit status.
////      else {
////
////      $params->target_url = $environment->url;
////      $params->description = t('Visit !url', array('!url' => $task->ref->environment->url));
////      $post_url = "/repos/$owner/$repo/deployments/{$deployment_id}/statuses";
////
////      drush_log('Attempting to create github deployment status: ' . $post_url, 'success');
////
////      $deployment_status = $client->getHttpClient()->post($post_url, json_encode($params));
//      if ($status == HOSTING_TASK_WARNING) {
//        $description = t('DevShop: !type success with warnings. [!url]', array(
//          '!url' => $environment->url,
//          '!type' => $task->task_type,
//        ));
//      }
//      else {
//        $description = t('DevShop: !type !status [!url]', array(
//          '!status' => $state,
//          '!url' => $environment->url,
//          '!type' => $task->task_type,
//        ));
//      }
//
//      $sha = devshop_environment_get_git_sha($environment->repo_path);
//
//      // Update the log URL because we know the Task NID now.
//      $task_url = url("node/$task->nid", array(
//        'absolute' => TRUE,
//      ));
//
//      // If task node really is NEW, aka called from hosting_task_insert(), create new deployment.
//      $task_github_deployment_id = db_select('hosting_task_arguments', 'a')
//        ->fields('a', array(
//          'value',
//        ))
//        ->condition('name', 'github_deployment_id')
//        ->condition('vid', $task->vid)
//        ->execute()
//        ->fetchField()
//      ;
//
//      // No Deployment ID. Create deployment.
//      if (empty($task_github_deployment_id)) {
//        watchdog('devshop_github', 'NEW TASK, NEW DEPLOYMENT: Task NID: ' . $task->nid);
//        $deployment_object = DevShopGitHubApi::createDeployment($environment, $state, $description, $sha, $task_url);
//        $deployment_id = $deployment_object->id;
//        $key = 'github_deployment_id';
//        $value = $deployment_object->id;
//        $id = db_insert('hosting_task_arguments')
//          ->fields(array(
//            'vid' => $task->vid,
//            'nid' => $task->nid,
//            'name' => $key,
//            'value' => $value,
//          ))
//          ->execute();
//        watchdog('devshop_github', 'Deployment saved to hosting_task_arguments: Task NID: ' . $task->nid);
//      }
//      else {
//        $deployment_id = $task_github_deployment_id;
//      }
//
//      // If existing github deployment ID is specified in the task_args, use that.
//      if (!empty($deployment_id)) {
//        $deployment_object = DevShopGitHubApi::createDeployment($environment, $state, $description, $sha, $task_url, $deployment_id);
//      }
//      else {
//        $deployment_object = DevShopGitHubApi::createDeployment($environment, $state, $description, $sha, $task_url);
//      }
//      drush_log('RETURNED FROM DevShopGitHubApi::createDeployment:  ' . print_r($deployment_object, 1), 'p_log');
//
//      // If deploy task fails, tests won't run.
//      if ($environment->settings->deploy['test'] && $status == HOSTING_TASK_ERROR) {
////
////        $params = new stdClass();
////        $params->state = $state;
////          $params->description = t('DevShop: Tests not run due to Deploy Fail');
////        $params->context = "devshop/{$project->name}/tests";
////
////        $deployment_status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
////        drush_log('Commit status created for devshop/tests', 'success');
//
//      }
}
