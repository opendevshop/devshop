<?php

namespace DevShop\Component\GitRemoteMonitor;

use Symfony\Component\String\Slugger\AsciiSlugger;

/**
 *
 */
class GitRemote
{
    /**
     * @var Task
     */
    public $task;

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
        $output = implode(PHP_EOL, $references);

      // Only load refs if exit was successful.
        if ($exit == 0) {
            return $output;
        }
        // Exit 1 means no new references
        elseif ($exit == 1) {
            $this->task->daemon->log("No new references found for $this->url");
            return NULL;
        }
        else {
            $message = "git-remote-monitor references:diff exited with $exit. Output: 
$output";
            $this->task->daemon->fatal_error($message);
            return NULL;
        }
    }

    /**
     * Remove characters and return a "slug" that can be used for a filename.
     * @param $url
     *
     * @return string
     */
    static public function getSlug($url) {
        $slugger = new AsciiSlugger();
        return (string) $slugger->slug($url);
    }
}
