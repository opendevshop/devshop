<?php

namespace DevShop\Component\GitRemoteMonitor;

use Robo\Tasks;
use Symfony\Component\Process\Exception\LogicException;
use Symfony\Component\Process\Exception\ProcessFailedException;
use Symfony\Component\Process\Exception\ProcessSignaledException;
use Symfony\Component\Process\Exception\ProcessTimedOutException;
use Symfony\Component\Process\Process;
use Symfony\Component\Yaml\Yaml;

class Commands extends Tasks
{

  /**
   * Print the list of remotes to monitor, one per line. Derived from config "remotes" or "remotes_callback".
   */
  public function remotes() {
    $remotes = $this->getRemotes();
    echo implode(PHP_EOL, $remotes);
    return 0;
  }

  /**
   * Get an array of remotes from grm config.
   * @return string[]
   */
  public function getRemotes() {
    /** @var Robo\Config\Config $config */
    $config = $this->getContainer()->get('config');

    // Look for remotes
    // GRM_REMOTES
    $remotes = $config->get('remotes');

    // GRM_REMOTES_CALLBACK
    if (empty($remotes)) {
      // Execute remotes.callback to return list of remotes.
      $callback = $config->get('remotes_callback');

      if (empty($callback)) {
        throw new LogicException('No "remotes" or "remotes_callback" configuration options found. At least one config value must be set.');
      }
      $remotes = trim(shell_exec($callback));
      if (empty($remotes)) {
        throw new LogicException("Remote callback ($callback) returned nothing.");
      }
    }

    // Always return an array.
    if (is_string($remotes)) {
      $remotes = explode(PHP_EOL, $remotes);
    }
    return $remotes;
  }

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
   * This the current references for this remote.
   *
   * @arg $git_remote The URL of the remote repository.
   * @option timeout The length of time to let the process run until timeout.
   */
  public function references($git_remote, $opts = [
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

