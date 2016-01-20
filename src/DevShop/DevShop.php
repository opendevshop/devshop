<?php

/*
 * This file is part of DevShop.
 *
 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop;

/**
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class DevShop
{
  const VERSION = '@package_version@';
  const BRANCH_ALIAS_VERSION = '@package_branch_alias_version@';
  const RELEASE_DATE = '@release_date@';

  /**
   * @var Config
   */
  private $config;


  /**
   * @param Config $config
   */
  public function setConfig(Config $config)
  {
    $this->config = $config;
  }

  /**
   * @return Config
   */
  public function getConfig()
  {
    return $this->config;
  }
}