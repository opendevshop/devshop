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
   * @command whoami
   */
  public function whoami()
  {

    /**
     * @var \Github\Api\CurrentUser
     */
    $user = $this->cli->api('me')->show();

    foreach ($user as $name => $value) {
      if (!is_array($value)) {
        $rows[] = [$name, $value];
      }
    }
    $this->io()->table(['Name', 'Value'], $rows);
    return 0;
  }
}
