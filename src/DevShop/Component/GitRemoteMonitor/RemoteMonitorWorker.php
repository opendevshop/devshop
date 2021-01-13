<?php

namespace DevShop\Component\GitRemoteMonitor;

use Symfony\Component\Process\Exception\ProcessSignaledException;

class RemoteMonitorWorker implements \Core_IWorker
{
  /**
   * Git remote URL.
   * @var String
   */
  private $url;

  function check_environment()
  {
    // TODO: Implement check_environment() method.
  }
  function setup()
  {
    // TODO: Implement setup() method.
    $this->log('DaemonWorker setup');
    $this->url = 'remote URL?';
  }
  public function teardown()
  {
    // TODO: Implement teardown() method.
  }

  /**
   * Read references from remote.
   * @return Array    Return associative array of results
   */
  public function getRemotes(Array $existing_results)
  {
    static $calls = 0;
    $calls++;

    $this->results = $existing_results;
    $this->mediator->log('Running git ls-remote...');

    $command = "git ls-remote {$this->url}";
    $process = new Process($command);

    try {
      $process->mustRun();
      $this->results[$this->url] = $process->getOutput();
    }
    catch (ProcessSignaledException $e) {
      $this->io()->error("Command signalled.");
      $this->io()->write($e->getMessage());
      return $process->getExitCode();
    }
    catch (ProcessTimedOutException $e) {
      $this->io()->error("Command timed out.");
      $this->io()->write($e->getMessage());
      return $process->getExitCode();
    }
    catch (ProcessFailedException $e) {
      $this->io()->error("Command failed.");
      $this->io()->write($e->getMessage());
      return $process->getExitCode();
    }
    return $this->results;
  }
}
