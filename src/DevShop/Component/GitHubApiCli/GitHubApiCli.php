<?php

namespace DevShop\Component\GitHubApiCli;

use Github\Client as GitHubApiClient;

class GitHubApiCli
{
  protected $apiToken;

  /**
   * @var GitHubApiClient
   */
  private $apiClient;

    /**
     * @var string The "version" of the GitHub API to use.
     */
  private $apiVersion;

  /**
   * GitHubApiCli constructor
   *
   * @param $apiToken String GitHub API Token. If not passed, will look for
   *    GTIHUB_TOKEN environment variable.
   *
   * @param $apiVersion String GitHub API Version. If a feature requires an
   * additional "Accept" header like "application/vnd.github.flash-preview+json",
   * Specify the string between "github." and "+json".
   */
  public function __construct($apiToken = NULL, $apiVersion = 'flash-preview')
  {
    if (!$apiToken && getenv('GITHUB_TOKEN')) {
      $this->apiToken = getenv('GITHUB_TOKEN');
    }
    else {
      $this->apiToken = $apiToken;
    }

    $this->apiVersion = $apiVersion;

    // Setup GitHub API client
    $this->apiClient = new GitHubApiClient();
    // @TODO: Allow password auth?
    $this->apiClient->authenticate($this->getToken(), null, GitHubApiClient::AUTH_HTTP_TOKEN);

    // Set options or headers from CLI or config options.
    // @see Client::options
    // Using pattern from HttpClient::clearHeaders()
    $this->apiClient->setHeaders([
      'Accept' => sprintf('application/vnd.github.%s+json', $this->apiVersion),
    ]);

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

  /**
   * Return all available APIs.
   *
   * This has to be a list of strings because the GitHub\Client::api method uses
   * a long "switch" statement to map API strings to classes.
   *
   * @return array List of available APIs.
   */
  public function getApis()
  {
    $apis[] = 'me';
    $apis[] = 'deployment';
    $apis[] = 'enterprise';
    $apis[] = 'git';
    $apis[] = 'git_data';
    $apis[] = 'gist';
    $apis[] = 'issue';
    $apis[] = 'markdown';
    $apis[] = 'notification';
    $apis[] = 'organization';
    $apis[] = 'pull_request';
    $apis[] = 'rate_limit';
    $apis[] = 'repo';
    $apis[] = 'search';
    $apis[] = 'team';
    $apis[] = 'user';
    $apis[] = 'authorization';
    $apis[] = 'meta';
    return $apis;
  }

  /**
   * Return all available APIs.
   *
   * This has to be a list of strings because the GitHub\Client::api method uses
   * a long "switch" statement to map API strings to classes.
   *
   * @return array List of available APIs.
   */
  public function getApisWithAliases()
  {
    $apis[] = 'me';
    $apis[] = 'current_user';
    $apis[] = 'currentUser';
    $apis[] = 'deployment';
    $apis[] = 'deployments';
    $apis[] = 'ent';
    $apis[] = 'enterprise';
    $apis[] = 'git';
    $apis[] = 'git_data';
    $apis[] = 'gitData';
    $apis[] = 'gist';
    $apis[] = 'gists';
    $apis[] = 'issue';
    $apis[] = 'issues';
    $apis[] = 'markdown';
    $apis[] = 'notification';
    $apis[] = 'notifications';
    $apis[] = 'organization';
    $apis[] = 'organizations';
    $apis[] = 'pr';
    $apis[] = 'pullRequest';
    $apis[] = 'pull_request';
    $apis[] = 'pullRequests';
    $apis[] = 'pull_requests';
    $apis[] = 'rateLimit';
    $apis[] = 'rate_limit';
    $apis[] = 'repo';
    $apis[] = 'repos';
    $apis[] = 'repository';
    $apis[] = 'repositories';
    $apis[] = 'search';
    $apis[] = 'team';
    $apis[] = 'teams';
    $apis[] = 'user';
    $apis[] = 'users';
    $apis[] = 'authorization';
    $apis[] = 'authorizations';
    $apis[] = 'meta';
    return $apis;
  }

  /**
   * Return all available = methods for the chosen API.
   *
   * @param $apiName
   */
  public function getApiMethods($apiName) {
    return $this->getPublicMethods($this->api($apiName));
  }

  /**
   * Return all public methods that are not magic methods.
   *
   * @param $classNameOrInstance
   */
  public function getPublicMethods($classNameOrInstance) {

    // Ignore special functions, such as __construct and __call, which
    // can never be commands.
    $commandMethodNames = array_filter(
      get_class_methods($classNameOrInstance) ?: [],
      function ($m) use ($classNameOrInstance) {
        $reflectionMethod = new \ReflectionMethod($classNameOrInstance, $m);
        return !$reflectionMethod->isStatic() && !preg_match('#^_#', $m);
      }
    );

    return $commandMethodNames;
  }
}
