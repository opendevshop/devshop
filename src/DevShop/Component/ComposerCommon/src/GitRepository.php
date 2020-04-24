<?php

namespace DevShop\Component\Common;

use TQ\Git\Repository\Repository;

class GitRepository extends Repository {

  /**
   * Run a git command in the repository directory.
   *
   * @param   string  $method             The VCS command, e.g. show, commit or add
   * @param   array   $arguments          The command arguments.
   * @return  CallResult
   * @example $this->callGit('status', array('--short'))
   */
  public function callGit($command, $arguments = array(), $message = "Git command failed.") {
    /** @var $result CallResult */
    $result = $this->getRepository()->getGit()->{$command}($this->getRepository()->getRepositoryPath(), $arguments);
    $result->assertSuccess($message);
    return $result;
  }
}