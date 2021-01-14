<?php

namespace DevShop\Component\GitRemoteMonitor;

/**
 *
 */
class Task implements \Core_ITask
{

  /**
   * @var Daemon
   */
    public $daemon;

  /**
   * @var \DevShop\Component\GitRemoteMonitor\GitRemote
   */
    public $gitRemote;

  /**
   * GitRemote constructor.
   *
   * @param $url
   */
    public function __construct($url)
    {
        $this->gitRemote = new GitRemote($url);
        $this->gitRemote->task = $this;
    }

  /**
   * This is called after setup() returns
   * @return void
   */
    public function start()
    {
        $has_new_refs = $this->gitRemote->poll();
        if (empty($has_new_refs)) {
          // Don't clog up logs, until we have debug mode capabilities.
            // $this->daemon->log("No new references found for {$this->gitRemote->url}");
            return;
        } else {
            $this->daemon->log('-------------------------');
            $this->daemon->log("New references detected in {$this->gitRemote->url}: ");
            $this->daemon->log($has_new_refs);
        }
    }

  /**
   * Called on Construct or Init
   * @return void
   */
    public function setup()
    {
        $this->daemon = Daemon::getInstance();
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
}
