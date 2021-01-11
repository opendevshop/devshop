<?php

namespace DevShop\Component\GitRemoteMonitor;

use Robo\Tasks;
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
}

