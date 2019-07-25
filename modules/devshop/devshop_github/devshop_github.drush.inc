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
  print "Git Reference: " . d($alias)->platform->git_ref;

  $project_name = d($alias)->project;
  $environment_name = d($alias)->environment;
  $project_node = devshop_projects_load_by_name($project_name);

  $environment = $project_node->project->environments[$environment_name];
  $environment->project = $project_node->project;

  DevShopGitHubApi::createDeployment($environment, $state, $description);

}
/**
 * Pass github token variable to the back end.
 *
 * @param $task
 */
function drush_devshop_github_pre_hosting_task($task)
{
  // @TODO If task is install or devshop-deploy, erase the deployment status from the PR so a new one is created.
  // @TODO: We should erase and create a "pending" status when the task nodes are created.

  drush_set_option('devshop_github_token', variable_get('devshop_github_token', null));
}

/**
 * Implements hook_hosting_task_update_status().
 */
function devshop_github_hosting_task_update_status($task, $status) {

  // React only on certain task types and if project and environment exist.
  $types = array('install', 'test', 'devshop-deploy');
  if (in_array($task->task_type, $types) && isset($task->ref->project) && !empty($task->ref->project) && isset($task->ref->environment) && !empty($task->ref->environment)) {
    $project = $task->ref->project;
    $environment = $task->ref->environment;
  }
  else {
    return;
  }

  // If a pull request object and Deployment is available...
  if (isset($environment->github_pull_request->pull_request_object->deployment)) {

    // If project is configured to reinstall every time, only react on "install" tasks. Otherwise, we get two github deployments because both a "deploy" (git pull) and a "install" task are run on each git push.
    if ($project->settings->github['pull_request_reinstall'] && $task->task_type == 'devshop-deploy') {
      return;
    }

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
        $params->target_url = url("node/{$task->nid}/view", array('absolute' => TRUE));

        if ($status == HOSTING_TASK_WARNING) {
          $params->description = t('DevShop: Tests passed with warnings');
        }
        else {
          $params->description = t('DevShop: Tests !status!', array('!status' => $state));
        }
        $params->context = "devshop/{$project->name}/tests";

        $status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
        drush_log('Commit status created for devshop/tests!', 'success');
      }
      // Otherwise we create a deployment and a commit status.
      else {

      $params->target_url = $environment->url;
      $params->description = t('Visit !url', array('!url' => $task->ref->environment->url));
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
        $params->description = t('DevShop: Deploy success with warnings. [!url]', array(
          '!url' => $environment->url,
        ));
      }
      else {
        $params->description = t('DevShop: Deploy !status [!url]', array(
          '!status' => $state,
          '!url' => $environment->url,
        ));
      }
      $params->context = "devshop/{$project->name}/deploy";

      $deployment_status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
      drush_log('Commit status created!', 'success');

        // If deploy task fails, tests won't run.
        if ($environment->settings->deploy['test'] && $status == HOSTING_TASK_ERROR) {

          $params = new stdClass();
          $params->state = $state;
            $params->description = t('DevShop: Tests not run due to Deploy Fail');
          $params->context = "devshop/{$project->name}/tests";

          $deployment_status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
          drush_log('Commit status created for devshop/tests', 'success');
        }
      }
    } catch (Github\Exception\RuntimeException $e) {
      drush_log('GitHub API Error: ' . $e->getMessage(), 'error');
    }
  }
}
