<?php

namespace DevShop\Component\GitRemoteMonitor;

/**
 *
 */
class GitRemote implements \Core_ITask
{
  /**
   * Git remote URL
   * @var String
   */
    private $url;

  /**
   * String result of git ls-remote
   * @var array
   */
    private $references = [];

  /**
   * A handle to the Daemon object
   * @var \Core_Daemon
   */
    public $daemon = null;

  /**
   * GitRemote constructor.
   *
   * @param $url
   */
    public function __construct($url)
    {
        $this->url = $url;
    }

  /**
   * This is called after setup() returns
   * @return void
   */
    public function start()
    {
        $has_new_refs = $this->poll();
        if (!$has_new_refs) {
          // Don't clog up logs.
          //      $this->daemon->log("No new references found for $this->url");
            return;
        } else {
            $this->daemon->log('-------------------------');
            $this->daemon->log("New references detected in $this->url: ");
            $this->daemon->log($has_new_refs);
        }
    }

  /**
   * Called on Construct or Init
   * @return void
   */
    public function setup()
    {
        $this->daemon = RemoteMonitorDaemon::getInstance();
    }

  /**
   * Called on Destruct
   * @return void
   */
    public function teardown()
    {
    }

  /**
   * This is called during object construction 2to validate any dependencies
   * @return Array    Return array of error messages (Think stuff like "GD Library Extension Required" or
   *                  "Cannot open /tmp for Writing") or an empty array
   */
    public function check_environment()
    {
        $errors = array();
        return $errors;
    }

  /**
   * Poll the remote for updated information -- Simulate an API call of varying duration.
   * @return string Output from git ls-remote.
   */
    public function poll()
    {
        static $calls = 0;
        $calls++;

        if (empty($this->url)) {
            throw new \Exception('Remote URL cannot be empty.');
        }
        $references = [];
        $exit = 0;
        exec("./git-remote-monitor references:diff {$this->url}", $references, $exit);

      // Only load refs if exit was successful.
        if ($exit == 0) {
            return implode(PHP_EOL, $references);
        }
    }
}
