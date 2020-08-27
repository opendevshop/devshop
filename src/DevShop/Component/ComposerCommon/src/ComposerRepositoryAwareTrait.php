<?php

namespace DevShop\Component\Common;

use Eloquent\Composer\Configuration\ConfigurationReader;
use Eloquent\Composer\Configuration\Element\Configuration;

trait ComposerRepositoryAwareTrait
{
  use GitRepositoryAwareTrait;

  /**
   * @var Configuration
   */
  protected $composerConfig = NULL;

  /**
   * @param Configuration $config If left empty, Configuration object will be loaded from composer.json from the repository root.
   *
   * @return $this
   */
  public function setComposerConfig(Configuration $configuration = NULL)
  {
    $reader = new ConfigurationReader;
    if ($configuration) {
      $this->composerConfig = $configuration;
    }
    else {
      $this->composerConfig =  $reader->read($this->getRepository()->getRepositoryPath() . '/composer.json');
    }

    return $this;
  }

  /**
   * @return Configuration
   */
  public function getComposerConfig()
  {
    if (!$this->composerConfig){
      $this->setComposerConfig();
    }
    return $this->composerConfig;
  }

}
