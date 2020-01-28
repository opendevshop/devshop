<?php

namespace DevShop\Environment;

use DevShop\Environment\Environment;
use Symfony\Component\Process\Process;

class EnvironmentGitInfo {

  /**
   * @var Environment
   */
  public $environment;

  /**
   * EnvironmentGitInfo constructor.
   *
   * @param $environment Environment
   */
  function __construct($environment) {
    if (get_class($environment) == "stdClass") {
      $this->environment = new Environment($environment);
    }
    else {
      $this->environment = $environment;
    }
  }

  public function refType() {
    switch (TRUE) {
      case '':


    }

  }

  /**
   * Run a command in this environments directory and return the results.
   * @param $cmd string Shell command to run
   */

  /**
   * Run a command in this environments directory and return the results.
   *
   * @param string $command
   *   The shell command to run in the environment's directory.

   * @param string $return
   *   Set to 'lines' to return lines as an array.
   *
   * @return array|string
   */
  public function process(string $cmd, string $return = 'string') {
    $process = new Process($cmd);
    $process->setWorkingDirectory($this->environment->repo_path);
    $process->setTimeout(null);
    $output = [];

    $process->mustRun(function ($type, $buffer) use ($output) {
      $output[] = $buffer;
    });

    if ($return == 'lines') {
      return $output;
    }
    else {
      return implode('', $output);
    }
  }
}