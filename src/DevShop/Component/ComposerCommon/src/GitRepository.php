<?php

namespace DevShop\Component\Common;

use TQ\Git\Repository\Repository;
use TQ\Vcs\Cli\CallResult;

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

  /**
   * Returns TRUE if the working directory is in 'detached HEAD' state.
   * https://stackoverflow.com/questions/52221558/programmatically-check-if-head-is-detached
   *
   * @return  boolean
   */
  public function isDetached() {
    /** @var $result CallResult */
    $result = $this->getGit()->{'symbolic-ref'}($this->getRepositoryPath(), array(
      '-q',
      'HEAD'
    ));

    // If command failed, HEAD is not on a branch, aka detached.
    return 0 !== $result->getReturnCode();
  }
}