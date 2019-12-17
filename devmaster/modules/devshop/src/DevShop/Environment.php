<?php

namespace DevShop\Environment;

/**
 * Class Environment
 *
 * This is a temporary bridge to full OOP.
 *
 * I needed to centralize Git information and I just couldn't take it anymore.
 *
 * @package DevShop\Environment
 */
class Environment {

  /**
   * @var \stdClass A legacy DevShop Environment stdClass.
   */
  protected $data;

  /**
   * @var \DevShop\Environment\EnvironmentGitInfo
   */
  public $git;

  /**
   * Environment constructor.
   *
   * @param $data A legacy DevShop Environment stdClass.
   */
  function __construct($data) {
    $this->data = $data;
    $this->git = new EnvironmentGitInfo($this);
  }

  /**
   * Return properties from $this->data->$name when requested from $this->$name
   *
   * @param $name
   *
   * @return mixed
   */
  function __get($name) {
    return $this->data->{$name};
  }

  /**
   * Set properties from $this->data->$name when requested from $this->$name
   *
   * @param $name
   *
   * @return mixed
   */
  function __set($name, $value) {
    $this->data->{$name} = $value;
  }

  /**
   * Check properties from $this->data->$name when requested from $this->$name
   *
   * @param $name
   *
   * @return bool
   */
  public function __isset($name)
  {
    return isset($this->data->{$name});
  }

  /**
   * Unset properties from $this->data->$name when requested from $this->$name
   *
   * @param $name
   *
   * @return bool
   */
  public function __unset($name)
  {
    unset($this->data->{$name});
  }
}