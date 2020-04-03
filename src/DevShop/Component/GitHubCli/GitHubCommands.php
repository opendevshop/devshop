<?php

namespace DevShop\Component\GitHubCli;

class GitHubCommands extends \Robo\Tasks
{

  /**
   * @var \DevShop\Component\GitHubCli\GitHubCli
   */
  protected $cli;

  /**
   * GitHubCommands constructor.
   */
  public function __construct() {
    $this->cli = new GitHubCli();
  }


  /**
   * Send an API request.
   *
   * @command api
   *
   * @param $apiName string The name of the specific API to use. See https://github.com/KnpLabs/php-github-api/blob/master/lib/Github/Client.php#L166 for available options.
   * @param $apiMethod string The API method to call. Depends on the API used. Common methods include show, create, update, remove. See the available AbstractAPI classes at https://github.com/KnpLabs/php-github-api/tree/master/lib/Github/Api.
   * @param $apiMethodArgs string All additional arguments are passed to the apiMethod.
   *
   * @see \Github\Client
   * @see \Github\Client::api()
   */
  public function api($apiName, $apiMethod = 'show', array $apiMethodArgs)
  {
     $api = $this->cli->api($apiName);

     // When using call_user_func_array on objects, the first $para_arr must be the object method.
     $object = call_user_func_array(array($api, $apiMethod), $apiMethodArgs);

     $this->io()->table(['Name', 'Value'], $this->objectToTableRows($object));
  }

  /**
   * Show the data for the currently authenticated user. (The owner of the token.)
   *
   * @command whoami
   */
  public function whoami()
  {

    /**
     * @var \Github\Api\CurrentUser
     */
    $user = $this->cli->api('me')->show();

    // @TODO: Add a "format" option to return json, yml, or pretty
    $this->io()->table(['Name', 'Value'], $this->objectToTableRows($user));
    return 0;
  }

  /**
   * Prepare an object for display in the CLI.
   * @param $obj
   *
   * @return array
   */
   private function objectToTableRows($obj) {
    $rows = [];
    foreach ($obj as $name => $value) {
      if (!is_array($value)) {
        $rows[] = [$name, $value];
      }
    }
    return $rows;
  }
}
