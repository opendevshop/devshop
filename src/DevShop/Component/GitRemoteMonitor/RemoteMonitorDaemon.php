<?php

namespace DevShop\Component\GitRemoteMonitor;

use Robo\Common\IO;

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
  protected $loop_interval = 1;

  /**
   * The list of git remoteremotes and their ls-remote output.
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
    $this->log('Setup');
  }

  /**
   * This is where you implement the tasks you want your daemon to perform.
   * This method is called at the frequency defined by loop_interval.
   *
   * @return void
   */
  protected function execute()
  {
    $this->log('Git Remote Monitor Daemon');
    $this->task([$this, 'executeTask']);
  }

  protected function executeTask()
  {
    $this->log('Execute task then sleep for 20...');
    $this->log('Execute task then sleep for 20...');

    sleep(20);
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
