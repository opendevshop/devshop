<?php
//
//
//class DevShopGitHubApi {
//
//  /**
//   * @var \Github\Client
//   */
//  public $client;
//
//  /**
//   * @var stdClass
//   */
//  public $environment;
//
//  public function __construct()
//  {
//    try {
//      $this->client = devshop_github_client();
//    }
//    catch (\Exception $e) {
//      throw $e;
//    }
//  }
//
//  /**
//   * Creates a GitHub "Deployment" and "Deployment Status".
//   *
//   * If the $task does not already have a github deployment in the database,
//   * it will create a new one on GitHub. If it does, it will be loaded.
//   *
//   * Once the deployment has been loaded or created, this method sets a
//   * "GitHub Deployment Status".
//   *
//   * @see https://developer.github.com/v3/repos/deployments/#create-a-deployment
//   * @see https://developer.github.com/v3/repos/deployments/#create-a-deployment-status
//   *
//   * @param $environment
//   *   An devshop environment object.
//   *
//   * @param $state
//   *   The state of the deployment status. Options are:
//   *   error, failure, pending, in_progress, queued, or success
//   *
//   * @param string $sha
//   *   A specific git commit SHA, if desired. If left empty, deployment will be
//   *   made against the environment "git_ref", a branch or tag.
//   * @param $log_url
//   *
//   * @return $deployment_object
//   *   A deployment object returned from GitHub.
//   */
//  static function deploy($environment, $state = 'pending', $task, $description = NULL, $sha  = NULL, $log_url = NULL) {
//
//    if (empty($task->nid)) {
//      return FALSE;
//    }
//
//    $project_node = node_load($environment->project_nid);
//    $project = $project_node->project;
//
//    $hostmaster_uri = hosting_get_hostmaster_uri();
//
//    // Lookup existing deployment for this task. I used hook_node_load() to ensure it's there, but who knows? data isn't there sometimes. Thanks, Drupal!
//    $existing_deployment_object =  unserialize(db_query('SELECT deployment_object FROM {hosting_devshop_github_deployments} WHERE task_nid = :nid', array(':nid' => $task->nid))->fetchField());
//
//    // If Deployment doesn't already exist, create it.
//    try {
//      $client = devshop_github_client();
//
//      // Git Reference. Use sha if specified.
//      $owner = $environment->github_owner;
//      $repo = $environment->github_repo;
//
//      if (!empty($environment->github_pull_request)) {
//        $ref = $environment->github_pull_request->pull_request_object->head->ref;
//        $owner = $project->github_owner;
//        $repo = $project->github_repo;
//      }
//      else {
//        $ref = $environment->git_ref;
//      }
//
//      if (empty($existing_deployment_object)) {
//        watchdog('devshop_github', 'Creating new GitHub Deployment for Task !nid', array('!nid' => $task->nid));
//        $deployment = new stdClass();
//
//        // If there is a PR, we must use the SHA in it, so that forked repos properly get deployment status.
//        if ($sha) {
//          $ref = $sha;
//        }
//
//        $deployment->ref = $ref;
//
//        // Set auto_merge based on project settings.
//        $deployment->auto_merge = (bool) $project->settings->github['pull_request_auto_merge'];
//
//        // https://developer.github.com/v3/repos/deployments/#create-a-deployment
//        $deployment->task = "deploy:{$task->task_type}";
//
//        // In GitHub's API, "environment" is just a small string it displays on the pull request:
//        // It's a better  UX to show the full URI in the "environment" field.
//        $deployment->environment = "{$environment->uri} - {$task->task_type}";
//        $deployment->payload = array(
//          'devshop_site_url' => $environment->dashboard_url,
//          'devmaster_url' => $hostmaster_uri,
//        );
//        // Deployment description is limited to
//        $deployment->description = substr(t('Deploying ref !ref to environment !env for project !proj: !link [by !server]', array(
//          '!ref' => $deployment->ref,
//          '!env' => $environment->name,
//          '!project' => $project->name,
//          '!link' => $deployment->environment,
//          '!server' => $hostmaster_uri,
//        )), 0, 140);
//        $deployment->required_contexts = array();
//
//        // @TODO: Use the developer preview to get this flag: https://developer.github.com/v3/previews/#enhanced-deployments
//  #      $deployment->transient_environment = true;
//
//        // @TODO: Support deployment notifications for production.
//  #      $deployment->production_environment = false;
//
//        // Create Deployment
//        $post_url = "/repos/$owner/$repo/deployments";
//
//        // @TODO: Detect merged branch response message and log it.
//        $deployment_object = json_decode($client->getHttpClient()->post($post_url, array(), json_encode($deployment))->getBody(TRUE));
//
//        $deployment_data = self::saveDeployment($deployment_object, $task->nid);
//        $deployment_object = $deployment_data->deployment_object;
//        watchdog('devshop_github', 'New Deployment created: ' . json_encode($deployment_object));
//
//        // @TODO: Run another git fetch! There might be new commits after the Deployment got created if there was an auto_merge.
//      }
//      // GitHub Deployment found attached to task, use that. Do not create new deployment status.
//      else {
//          $deployment_object = $existing_deployment_object;
//          watchdog('devshop_github', 'Existing Deployment loaded: ' . json_encode($deployment_object));
//      }
//    }
//    catch (Github\Exception\RuntimeException $e) {
//      watchdog('devshop_github', "GitHub Error: {$e->getMessage()} | Code: {$e->getCode()} | Post URL: $post_url");
//      // Code 409 is used when the GitHub Deployments API "Auto-merge" fails because there is a conflict.
//      // Instead of breaking our process by not getting a Deployment Object, try to save it again without auto_merge property.
//      if ((string) $e->getCode() == '409') {
//        $deployment->auto_merge = false;
//        $description .= ' ' . t('WARNING: Auto-Merge Failed. Deployed without updated primary branch.');
//
//        $deployment_object = json_decode($client->getHttpClient()->post($post_url, array(), json_encode($deployment))->getBody(TRUE));
//
//        $deployment_data = self::saveDeployment($deployment_object, $task->nid);
//        $deployment_object = $deployment_data->deployment_object;
//        watchdog('devshop_github', 'New Deployment created without auto_merge option: ' . $deployment_object->id);
//      }
//    }
//    catch (\Exception $e) {
//      watchdog('devshop_github', "GitHub Error: {$e->getMessage()} | Code: {$e->getCode()} | Post URL: $post_url | {$e->getTraceAsString()}");
//      return false;
//    }
//
//    // Deployment Status
//    $deployment_status = new stdClass();
//    $deployment_status->state = $state;
//
//    // Target URL is actually for the logs.
//    // According to GitHub API Docs:
//    // "This URL should contain output to keep the user updated while the task is running or serve as historical information for what happened in the deployment. "
//    $deployment_status->log_url =
//    $deployment_status->target_url =
//      empty($log_url)? $environment->dashboard_url: $log_url;
//
//    // @TODO: Generate a default description?
//    $deployment_status->description = $description;
//
//    // @TODO: Use developer preview to get this:
//    // https://developer.github.com/v3/previews/#deployment-statuses
//    // https://developer.github.com/v3/previews/#enhanced-deployments
//    $deployment_status->environment = $deployment_object->environment;
//    $deployment_status->environment_url = $environment->url;
//
//    // Create Deployment Status
//    try {
//      $post_url = "/repos/{$owner}/{$repo}/deployments/{$deployment_object->id}/statuses";
//      $deployment_status_data = json_decode($client->getHttpClient()->post($post_url, array(), json_encode($deployment_status))->getBody(TRUE));
//
//      watchdog('devshop_github', "Deployment status saved to $state: $deployment_status_data->id");
//      return $deployment_object;
//    }
//    catch (\Exception $e) {
//      watchdog('devshop_github', "Error sending Deployment Status to GitHub: {$e->getMessage()} | Code: {$e->getCode()} | Post URL: $post_url | {$e->getTraceAsString()}");
//      return false;
//    }
//  }
//
//  /**
//   * Save Deployment. Deployments never get updated.
//   *
//   * @param $project_nid
//   * @param $environment_name
//   * @param $deployment_id
//   */
//  public static function saveDeployment($deployment, $task_nid)  {
//
//    if (empty($task_nid)) {
//      return false;
//    }
//    $record = new stdClass();
//    $record->deployment_id = $deployment->id;
//    $record->task_nid = $task_nid;
//    $record->deployment_object = serialize($deployment);
//    drupal_write_record('hosting_devshop_github_deployments', $record);
//
//    // Unserialize and return.
//    $record->deployment_object = $deployment;
//    return $record;
//  }
//}
