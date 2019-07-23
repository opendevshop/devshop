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
   * Create a GitHub "Deployment" and "Deployment Status".
   *
   * @param $environment
   *   An environment object with "github_pull_request" property filled in.
   *
   * @param string $sha
   *   A specific git commit SHA, if desired. If left empty, deployment will be
   *   made against the environment "git_ref", a branch or tag.
   * @param $log_url
   */
  static function createDeployment($environment, $sha  = NULL, $log_url = NULL) {

    if (empty($environment->github_pull_request)) {
      throw new \Exception('No Pull Request data in this environment.');
    }

    $project = $environment->project;
    $environment->dashboard_url = url("node/{$environment->site}", array(
      'absolute' => true
    ));

    $deployment = new stdClass();

    // Git Reference. Use sha if specified.
    $deployment->ref = $sha? $sha: $environment->git_ref;

    // In GitHub's API, "environment" is just a small string it displays on the pull request:
    $deployment->environment = $project->name . '.' . $environment->name;

    $deployment->payload = array(
      'devshop_site_url' => $environment->dashboard_url,
      'devmaster_url' => $_SERVER['hostname'],
    );
    $deployment->description = t('DevShop Deployment');
    $deployment->required_contexts = array();

    // @TODO: Use the developer preview to get this flag: https://developer.github.com/v3/previews/#enhanced-deployments
    $deployment->transient_environment = true;

    // @TODO: Support deployment notifications for production.
    $deployment->production_environment = false;

    // Deployment Status
    $deployment_status = new stdClass();
    $deployment_status->state = 'queued';
    $deployment_status->target_url = $environment->url;
    $deployment_status->log_url = empty($log_url)? $environment->dashboard_url: $log_url;
    $deployment_status->description = t('New environment is being created.  Please stand by.');

    // @TODO: Use developer preview to get this:
    // https://developer.github.com/v3/previews/#deployment-statuses
    // https://developer.github.com/v3/previews/#enhanced-deployments
    $deployment_status->environment = $deployment->environment;
    $deployment_status->environment_url = $environment->url;
//    $message = '';
    try {
      $client = devshop_github_client();

      // Create Deployment
      $post_url = "/repos/$environment->github_owner/$environment->github_repo/deployments";
      $deployment_data = json_decode($client->getHttpClient()->post($post_url, json_encode($deployment))->getBody(TRUE));

      // Save Deployment object to PR
      $environment->github_pull_request->deployment = $deployment_data;

      // Create Deployment Status
      $post_url = "/repos/{$environment->github_owner}/{$environment->github_repo}/deployments/{$deployment_data->id}/statuses";
      $deployment_status_data = $client->getHttpClient()->post($post_url, json_encode($deployment_status));

      $environment->github_pull_request->deployment_status = $deployment_status_data;

    }
    catch (\Exception $e) {
      watchdog('devshop_github', 'GitHub Error: ' . $e->getMessage());
      return false;
    }
    catch (Github\Exception\RuntimeException $e) {
      watchdog('devshop_github', 'GitHub Error: ' . $e->getMessage());
      if ($e->getCode() == '409') {
//        $message .= "\n Branch is out of date! Merge code from base branch.";

        // Send a failed commit status to alert to developer
        $params = new stdClass();
        $params->state = 'failure';
        $params->target_url = $project->git_repo_url;
        $params->description = t('Branch is out of date! Merge from default branch.');
        $params->context = "devshop/{$project->name}/merge";

            // Post status to github
        $status = $client->getHttpClient()->post("/repos/$owner/$repo/statuses/$sha", json_encode($params));
      }

    } catch (Github\Exception\ValidationFailedException $e) {
      watchdog('devshop_github', 'GitHub Validation Failed Error: ' . $e->getMessage());
//      $message .= 'GitHub ValidationFailedException Error: ' . $e->getMessage();
    }
  }
}
