<?php

namespace DevShop\Component\GitRemoteMonitor;


class RemoteMonitorDaemon extends \Core_Daemon
{

  protected $install_instructions = [
    'Run as the user that will be cloning code.',
    'Create a writable folder at /var/log/git-remote-monitor.',
  ];

  /**
   * How many seconds to wait.
   * @var int
   */
  protected $loop_interval = 3;

  /**
   * The list of git remotes and their ls-remote output.
   * @var array
   */
  protected $remotes = array();

  /**
   * @param array $remotes
   */
  public function saveRemotes(Array $remotes) {
    $this->remotes = $remotes;
  }

  /**
   * The only plugin we're using is a simple file-based lock to prevent 2 instances from running
   */
  protected function setup_plugins()
  {
    $this->plugin('Lock_File');
  }

  /**
   * This is where you implement any once-per-execution setup code.
   * @return void
   * @throws \Exception
   */
  protected function setup()
  {
    $this->log('GitRemoteMonitor Setup');
  }

  /**
   * This is where you implement the tasks you want your daemon to perform.
   * This method is called at the frequency defined by loop_interval.
   *
   * @return void
   */
  protected function execute()
  {
    static $remotes_list_last = '';

    // Call git-remote-monitor remotes via shell, so that all of the Robo config is loaded and we don't have to integrate the remote daemon classes with robo classes.
    $remotes = [];
    exec('./git-remote-monitor remotes', $remotes, $exit);
    if ($exit != 0) {
      $this->fatal_error('git-remote-monitor remotes command failed: ' . implode(PHP_EOL, $remotes));
    }
    $count = count($remotes);
    $remotes_list = implode(PHP_EOL, $remotes);

    if ($remotes_list != $remotes_list_last) {
      $this->log("=========================");
      $this->log("Git Remote Monitor Daemon");
      $this->log("Currently Monitoring: $count ");
    }
    elseif (empty($count)) {
      $this->error("No remotes output from 'git-remote-monitor remotes' command.");
    }

    foreach ($remotes as $url) {
      $this->task(new GitRemote($url));
    }

    $remotes_list_last = $remotes_list;
  }

  /**
   * Dynamically build the file name for the log file. This simple algorithm
   * will rotate the logs once per day and try to keep them in a central /var/log location.
   * @return string
   */
  protected function log_file()
  {
    $dir = '/var/log/git-remote-monitor';
    if (@file_exists($dir) == false)
      @mkdir($dir, 0777, true);

    return $dir . '/log_' . date('Ymd');
  }
}
