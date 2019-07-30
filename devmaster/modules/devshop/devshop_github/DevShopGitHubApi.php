<?php


class DevShopGitHubApi {

  /**
   * @var \Github\Client
   */
  public $client;

  /**
   * @var stdClass
   */
  public $environment;

  public function __construct()
  {
    try {
      $this->client = devshop_github_client();
    }
    catch (\Exception $e) {
      throw $e;
    }
  }

  /**
   * Creates a GitHub "Deployment" and "Deployment Status".
   *
   * If the $task does not already have a github deployment in the database,
   * it will create a new one on GitHub. If it does, it will be loaded.
   *
   * Once the deployment has been loaded or created, this method sets a
   * "GitHub Deployment Status".
   *
   * @see https://developer.github.com/v3/repos/deployments/#create-a-deployment
   * @see https://developer.github.com/v3/repos/deployments/#create-a-deployment-status
   *
   * @param $environment
   *   An devshop environment object.
   *
   * @param $state
   *   The state of the deployment status. Options are:
   *   error, failure, pending, in_progress, queued, or success
   *
   * @param string $sha
   *   A specific git commit SHA, if desired. If left empty, deployment will be
   *   made against the environment "git_ref", a branch or tag.
   * @param $log_url
   *
   * @return $deployment_object
   *   A deployment object returned from GitHub.
   */
  static function deploy($environment, $state = 'pending', $task, $description = NULL, $sha  = NULL, $log_url = NULL) {

    if (empty($task->nid)) {
      return FALSE;
    }

//    $project = $environment->project;
    $hostmaster_uri = hosting_get_hostmaster_uri();

    // Lookup existing deployment for this task. I used hook_node_load() to ensure it's there, but who knows? data isn't there sometimes. Thanks, Drupal!
    $existing_deployment_object =  unserialize(db_query('SELECT deployment_object FROM {hosting_devshop_github_deployments} WHERE task_nid = :nid', array(':nid' => $task->nid))->fetchField());

    // If Deployment doesn't already exist, create it.
    try {
    $client = devshop_github_client();

    if (empty($existing_deployment_object)) {
      watchdog('devshop_github', 'Creating new GitHub Deployment for Task !nid', array('!nid' => $task->nid));
      $deployment = new stdClass();

      // Git Reference. Use sha if specified.
      // @TODO: Detect the actual SHA from git or the PR here?
      // NO PR SPECIFIC THINGS in Deployments.
      $deployment->ref = $sha? $sha: $environment->git_ref;

      // @TODO: Make this configurable.
      $deployment->auto_merge = false;

      // https://developer.github.com/v3/repos/deployments/#create-a-deployment
      $deployment->task = "deploy:{$task->task_type}";

      // In GitHub's API, "environment" is just a small string it displays on the pull request:
      // It's a better  UX to show the full URI in the "environment" field.
      $deployment->environment = $environment->uri;
      $deployment->payload = array(
        'devshop_site_url' => $environment->dashboard_url,
        'devmaster_url' => $hostmaster_uri,
      );
      // Deployment description is limited to
      $deployment->description = substr(t('Deploying ref !ref to environment !env for project !proj: !link [by !server]', array(
        '!ref' => $deployment->ref,
        '!env' => $environment->name,
        '!project' => $environment->project_name,
        '!link' => $deployment->environment,
        '!server' => $hostmaster_uri,
      )), 0, 140);
      $deployment->required_contexts = array();

      // @TODO: Use the developer preview to get this flag: https://developer.github.com/v3/previews/#enhanced-deployments
#      $deployment->transient_environment = true;

      // @TODO: Support deployment notifications for production.
#      $deployment->production_environment = false;

      // Create Deployment
      $post_url = "/repos/$environment->github_owner/$environment->github_repo/deployments";
      $deployment_object = json_decode($client->getHttpClient()->post($post_url, array(), json_encode($deployment))->getBody(TRUE));

      $deployment_data = self::saveDeployment($deployment_object, $task->nid);
      $deployment_object = $deployment_data->deployment_object;
      watchdog('devshop_github', 'New Deployment created: ' . $deployment_object->id);
    }
    // GitHub Deployment found attached to task, use that. Do not create new deployment status.
    else {
        $deployment_object = $existing_deployment_object;
        watchdog('devshop_github', 'Existing Deployment loaded: ' . $deployment_object->id);
    }

    // Deployment Status
    $deployment_status = new stdClass();
    $deployment_status->state = $state;

    // Target URL is actually for the logs.
    // According to GitHub API Docs:
    // "This URL should contain output to keep the user updated while the task is running or serve as historical information for what happened in the deployment. "
    $deployment_status->log_url =
    $deployment_status->target_url =
      empty($log_url)? $environment->dashboard_url: $log_url;

    // @TODO: Generate a default description?
    $deployment_status->description = $description;

    // @TODO: Use developer preview to get this:
    // https://developer.github.com/v3/previews/#deployment-statuses
    // https://developer.github.com/v3/previews/#enhanced-deployments
    $deployment_status->environment = $deployment_object->environment;
    $deployment_status->environment_url = $environment->url;

    // Create Deployment Status
    $post_url = "/repos/{$environment->github_owner}/{$environment->github_repo}/deployments/{$deployment_object->id}/statuses";
    $deployment_status_data = json_decode($client->getHttpClient()->post($post_url, array(), json_encode($deployment_status))->getBody(TRUE));

    watchdog('devshop_github', "Deployment status saved to $state: $deployment_status_data->id");
    }
    catch (Github\Exception\RuntimeException $e) {
      watchdog('devshop_github', "GitHub Error: {$e->getMessage()} | Code: {$e->getCode()} | Post URL: $post_url");
      if ((string) $e->getCode() == '409') {

        // @TODO: @ElijahLynn This deployment status is not sending, maybe because it is STILL out of date?
        // With devshop, we sent this warning as a commit status instead.
        watchdog('devshop_github', 'Caught github error: cannot merge code automatically. TODO: Send error commit status ...');
//        $deployment_object = DevShopGitHubApi::createDeployment($environment, 'error', $task->nid, t('GitHub cannot trigger a deployment: Branch needs manual merging from default branch. Error: !error', array(
//          '!error' => $e->getMessage(),
//        )));

        // @TODO: deployment_object is not getting set here. Return false.
        return false;
      }
    }
    catch (\Exception $e) {
      watchdog('devshop_github', "GitHub Error: {$e->getMessage()} | Code: {$e->getCode()} | Post URL: $post_url | {$e->getTraceAsString()}");
      return false;
    }

    return $deployment_object;
  }

  /**
   * Save Deployment. Deployments never get updated.
   *
   * @param $project_nid
   * @param $environment_name
   * @param $deployment_id
   */
  public static function saveDeployment($deployment, $task_nid)  {

    if (empty($task_nid)) {
      return false;
    }
    $record = new stdClass();
    $record->deployment_id = $deployment->id;
    $record->task_nid = $task_nid;
    $record->deployment_object = serialize($deployment);
    drupal_write_record('hosting_devshop_github_deployments', $record);

    // Unserialize and return.
    $record->deployment_object = $deployment;
    return $record;
  }
}
