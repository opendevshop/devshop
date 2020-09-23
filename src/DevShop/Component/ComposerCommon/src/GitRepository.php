<?php

namespace DevShop\Component\Common;

use TQ\Git\Repository\Repository;
use TQ\Vcs\Cli\CallException;
use TQ\Vcs\Cli\CallResult;

class GitRepository extends Repository
{
  const REMOTE_TRACKING_STATE_BEHIND = -1;
  const REMOTE_TRACKING_STATE_SYNCED = 0;
  const REMOTE_TRACKING_STATE_AHEAD  = 1;
  const REMOTE_TRACKING_STATE_DIVERGED  = 2;

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
   * Check ahead/behind state of this repository.
   *
   * @see https://stackoverflow.com/a/3278427
   * @return int
   */
  public function remoteTrackingState(){
    $local = $this->getLocalSha();
    $remote = $this->getRemoteSha();
    $base = $this->getMergeSha();

    if ($local == $remote) {
      return self::REMOTE_TRACKING_STATE_SYNCED;
    }
    elseif ($local == $base) {
      return self::REMOTE_TRACKING_STATE_BEHIND;
    }
    elseif ($remote == $base) {
      return self::REMOTE_TRACKING_STATE_AHEAD;
    }
    else {
      return self::REMOTE_TRACKING_STATE_DIVERGED;
    }
  }

  /**
   * Returns TRUE if the working directory has commits that have not been pushed to the remote.
   *
   * @return  boolean
   */
  public function isUpToDate()
  {
    return $this->remoteTrackingState() == self::REMOTE_TRACKING_STATE_SYNCED;
  }

  /**
   * Returns TRUE if the working directory has commits that have not been pushed to the remote.
   *
   * @return  boolean
   */
  public function isAhead()
  {
    return $this->remoteTrackingState() == self::REMOTE_TRACKING_STATE_AHEAD;
  }

  /**
   * Returns TRUE if the working directory has commits that have not been pushed to the remote.
   *
   * @return  boolean
   */
  public function isBehind()
  {
    return $this->remoteTrackingState() == self::REMOTE_TRACKING_STATE_BEHIND;
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