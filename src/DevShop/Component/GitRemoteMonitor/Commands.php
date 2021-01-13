<?php

namespace DevShop\Component\GitRemoteMonitor;

use Robo\Tasks;
use Symfony\Component\Process\Exception\ProcessFailedException;
use Symfony\Component\Process\Exception\ProcessSignaledException;
use Symfony\Component\Process\Exception\ProcessTimedOutException;
use Symfony\Component\Process\Process;
use Symfony\Component\Yaml\Yaml;

class Commands extends Tasks
{
  /**
   * Display the current state of the GitRemoteMonitor command, such as configuration.
   * @command status
   */
  public function status()
  {
    $this->io()->section('Git Remote Monitor');
    $this->io()->title('Status');
    return 0;
  }

  /**
   * Run the git-ls command and print out the results.
   *
   * @arg $git_remote The URL of the remote repository.
   * @option timeout The length of time to let the process run until timeout.
   */
  public function remoteList($git_remote, $opts = [
    'timeout' => 60,
  ]) {

    $command = "git ls-remote {$git_remote}";
    $process = new Process($command);
    $process->setTimeout($opts['timeout']);

    try {
      $process->mustRun();
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

    $this->io()->success("Retrieved references for {$git_remote}.");
    $this->io()->write($process->getOutput());

    // @TODO: Write to file.

    return 1;
  }
}

