<?php

namespace DevShop\Component\GitRemoteMonitor;

use Robo\Tasks;
use SebastianBergmann\Diff\Differ;
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
    return array_unique($remotes);
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
   * Show differences between last stored list of refs and the current list.
   *
   * @arg $git_remote The URL of the remote repository.
   * @option timeout The length of time to let the process run until timeout.
   *
   * @return int Returns 0 and prints the diff if current refs is different from the stored refs.
   */
  public function referencesDiff($git_remote, $opts = [
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

    // Save to file.
    $references = $process->getOutput();
    $config = $this->getContainer()->get('config');
    $yml_file_path = $_SERVER['HOME'] . '/.grm/remotes/' . $this->getDirectory($git_remote) . '.yml';
    $yml_file_dir = dirname($yml_file_path);

    if (!file_exists($yml_file_dir)) {
      mkdir($yml_file_dir, 0744, TRUE);
    }

    $file_path = $config->get('remotes_save_path', $_SERVER['HOME'] . '/.grm/remotes/' . $this->getDirectory($git_remote) . '.yml');

    // Compare to existing file.
    if (file_exists($file_path)) {
      $existing_refs = file_get_contents($file_path);
      if ($existing_refs == $references) {
        $this->io()->warning('No new references detected.');
        return 1;
      }
      else {

        $differ = new Differ();
        echo $differ->diff($existing_refs, $references);
        file_put_contents($file_path, $process->getOutput());
        return 0;
      }
    }
    else {
      $this->io()->warning('First scan. No new references.');
      file_put_contents($file_path, $process->getOutput());
      return 1;
    }
  }

  private function getDirectory($url) {
    // everything to lower and no spaces begin or end
    $url = strtolower(trim($url));

    //replace accent characters, depends your language is needed
    //$url=replace_accents($url);

    // decode html maybe needed if there's html I normally don't use this
    //$url = html_entity_decode($url,ENT_QUOTES,'UTF8');

    // adding - for spaces and union characters
    $find = array(' ', '&', '\r\n', '\n', '+',',');
    $url = str_replace ($find, '-', $url);

    //delete and replace rest of special chars
    $find = array('/[^a-z0-9\-<>]/', '/[\-]+/', '/<[^>]*>/');
    $repl = array('', '-', '');
    $url = preg_replace ($find, $repl, $url);

    //return the friendly url
    return $url;
  }
}