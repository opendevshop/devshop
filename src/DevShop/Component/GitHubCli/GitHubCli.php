<?php

namespace DevShop\Component\GitHubCli;

use Github\Client as GitHubClient;

class GitHubCli
{
  protected $apiToken;

  /**
   * @var GitHubClient
   */
  private $apiClient;

  /**
   * GitHubCLI constructor
   *
   * @param $apiToken String GitHub API Token. If not passed, will look for
   *    GTIHUB_TOKEN environment variable.
   */
  public function __construct($apiToken = NULL)
  {
    if (!$apiToken && getenv('GITHUB_TOKEN')) {
      $this->apiToken = getenv('GITHUB_TOKEN');
    }
    else {
      $this->apiToken = $apiToken;
    }

    // Setup GitHub API client
    $this->apiClient = new GitHubClient();
    // @TODO: Allow password auth?
    $this->apiClient->authenticate($this->getToken(), null, GitHubClient::AUTH_HTTP_TOKEN);

    // Set options or headers from CLI or config options.
    // @see Client::options

  }

  /**
   *
   */
  public function getToken() {
    return $this->apiToken;
  }

  /**
   * @TODO: Should this just extend GitHub\Client?
   *
   * @param $name
   */
  public function api($name) {
    return $this->apiClient->api($name);
  }
}
