<?php

namespace DevShop\Component\GitRemoteMonitor;

/**
 *
 */
class GitRemote
{
  /**
   * Git remote URL
   * @var String
   */
    public $url;

  /**
   * String result of git ls-remote
   * @var array
   */
    private $references = [];

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
