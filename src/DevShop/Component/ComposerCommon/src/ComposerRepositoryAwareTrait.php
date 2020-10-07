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
   * @var string The path to composer.json.
   */
  protected $composerPath = NULL;

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
      if (file_exists(getcwd() . '/composer.json' )) {
        $path = getcwd();
      }
      else {
        $path = $this->getRepository()->getRepositoryPath();
      }
      $this->composerConfig =  $reader->read($path . '/composer.json');
      $this->composerPath = $path;
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
