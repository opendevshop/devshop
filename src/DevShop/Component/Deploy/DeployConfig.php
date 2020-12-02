<?php
/**
 * @file Deploy.php
 *
 * Represents a single deploy action.
 */

namespace DevShop\Component\Deploy;

use Consolidation\Config\Config;
use DevShop\Component\Common\ComposerRepositoryAwareTrait;
use DevShop\Component\Common\GitRepository;
use DevShop\Component\Common\GitRepositoryAwareTrait;
use DevShop\Component\DeployStageGit;

class DeployConfig extends Config {

  use ComposerRepositoryAwareTrait;

  private $data;

  public function __construct(GitRepository $repo)
  {
    $this->setRepository($repo);
    if (file_exists($this->getRepository()->getRepositoryPath() . '/composer.json')) {
      $this->setComposerConfig();
    }
    // 0. @TODO: Allow ENV vars config to take precedence?
    // 1. Project composer.json extra.
    if (!empty($this->getComposerConfig()->extra()->deploy)) {
      $data = $this->getComposerConfig()->extra()->deploy;
    }
    // 2. @TODO: DeployConfigDefaultDrupal etc.
    // No config found? unlikely, but default to empty array.
    else {
      $data = [];
    }
    parent::__construct($data);
  }

  /**
   * Return all config data as an array.
   * @return array
   */
  function getAll() {
    return $this->config->export();
  }
}