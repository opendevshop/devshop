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
        exec("./git-remote-monitor diff {$this->url}", $references, $exit);
        $output = implode(PHP_EOL, $references);

      // Only load refs if exit was successful.
        if ($exit == 0) {
            return $output;
        } elseif ($exit == 1) {
            // Exit 1 means no new references
            // @TODO: Detect debug mode.
            // $this->task->daemon->log("No new references found for $this->url", 'debug');
            return '';
        } else {
            $message = "git-remote-monitor diff exited with $exit. Output: 
$output";
            $this->task->daemon->fatal_error($message, 'error');
        }
    }

    /**
     * Remove characters and return a "slug" that can be used for a filename.
     * @param $url
     *
     * @return string
     */
    public static function getSlug($url)
    {
        $slugger = new AsciiSlugger();
        return (string) $slugger->slug($url);
    }
}
