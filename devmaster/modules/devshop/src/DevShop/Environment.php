<?php

namespace DevShop\Environment;

/**
 * Class Environment
 *
 * @package DevShop\Environment
 */
class Environment {

  protected $environmentData;

  /**
   * @var \DevShop\Environment\EnvironmentGitInfo
   */
  public $git;

  /**
   * Environment constructor.
   *
   * @param $environment_data
   */
  function __construct($environment_data) {
    $this->environmentData = $environment_data;
    $this->git = new EnvironmentGitInfo($this);
  }

  /**
   * Return properties from environmentData when requested from $this
   *
   * @param $name
   *
   * @return mixed
   */
  function __get($name) {
    return $this->environmentData->$name;
  }

}