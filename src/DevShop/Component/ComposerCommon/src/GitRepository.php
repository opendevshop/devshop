<?php

namespace DevShop\Component\Common;

use TQ\Git\Repository\Repository;
use TQ\Vcs\Cli\CallException;
use TQ\Vcs\Cli\CallResult;

class GitRepository extends Repository
{

  /**
   * Run a git command in the repository directory.
   *
   * @param string $method The VCS command, e.g. show, commit or add
   * @param array $arguments The command arguments.
   *
   * @return  CallResult
   * @example $this->callGit('status', array('--short'))
   */
  public function callGit(
    $command,
    $arguments = [],
    $message = "Git command failed."
  ) {
    /** @var $result CallResult */
    $result = $this->getGit()->{$command}(
      $this->getRepositoryPath(),
      $arguments
    );
    $result->assertSuccess($message);

    return $result;
  }

  /**
   * Return the current SHA of the working copy clone of the repo.
   *
   * @return string
   */
  public function getLocalSha()
  {
    /** @var $result CallResult */
    $result = $this->callGit('rev-parse', ['@']);

    return $result->getStdOut();
  }

  /**
   * Return the current SHA of the upstream reference (origin).
   * Does NOT run git fetch.
   *
   * @return string
   */
  public function getRemoteSha()
  {
    /** @var $result CallResult */
    $result = $this->callGit('rev-parse', ['@{upstream}']);

    return $result->getStdOut();
  }

  /**
   * Return the current SHA
   *
   * @return string
   */
  public function getMergeSha()
  {
    /** @var $result CallResult */
    $result = $this->callGit('merge-base', ['@', '@{upstream}']);

    return $result->getStdOut();
  }

  /**
   * Returns TRUE if the working directory is in 'detached HEAD' state.
   * https://stackoverflow.com/questions/52221558/programmatically-check-if-head-is-detached
   *
   * @return  boolean
   */
  public function isDetached()
  {
    /** @var $result CallResult */
    $result = $this->getGit()->{'symbolic-ref'}(
      $this->getRepositoryPath(),
      [
        '-q',
        'HEAD'
      ]
    );

    // If command failed, HEAD is not on a branch, aka detached.
    return 0 !== $result->getReturnCode();
  }

  /**
   * Returns TRUE if the working directory has commits that have not been pushed to the remote.
   *
   * @return  boolean
   */
  public function isUpToDate()
  {
    return $this->getLocalSha() == $this->getRemoteSha();
  }

  /**
   * Returns TRUE if the working directory has commits that have not been pushed to the remote.
   *
   * @return  boolean
   */
  public function isAhead()
  {
    return $this->getRemoteSha() == $this->getMergeSha();
  }

  /**
   * Returns TRUE if the working directory has commits that have not been pushed to the remote.
   *
   * @return  boolean
   */
  public function isBehind()
  {
    return $this->getLocalSha() == $this->getMergeSha();
  }

  /**
   * @param string $git_reference
   * @param string[] $options
   *
   * @return string
   */
  public function showCommit(
    $git_reference = null,
    $args = ['--quiet', '--abbrev-commit', "--format=format:%h %s <%an> %cr"]
  ) {

    if ($git_reference) {
      $args[] = $git_reference;
    }

    try {
      /** @var $result CallResult */
      $result = $this->callGit('show', $args);
      return $result->getStdOut();
    }
    catch (CallException $exception) {
      throw $exception;
    }
  }
}