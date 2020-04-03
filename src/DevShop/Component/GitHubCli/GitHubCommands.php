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
   * @command api
   */
  public function api($apiName, $apiMethod = 'show', $arg1 = null, $arg2 = null,  $arg3 = null,  $arg4 = null, $opts = [])
  {
     $object = $this->cli->api($apiName)->{$apiMethod}($arg1);
     $this->io()->table(['Name', 'Value'], $this->objectToTableRows($object));
  }

  /**
   * @command whoami
   */
  public function whoami()
  {

    /**
     * @var \Github\Api\CurrentUser
     */
    $user = $this->cli->api('me')->show();
    $this->io()->table(['Name', 'Value'], $this->objectToTableRows($user));
    return 0;
  }

  /**
   * Prepare an object for display in the CLI.
   * @param $obj
   *
   * @return array
   */
  function objectToTableRows($obj) {
    $rows = [];
    foreach ($obj as $name => $value) {
      if (!is_array($value)) {
        $rows[] = [$name, $value];
      }
    }
    return $rows;
  }
}
