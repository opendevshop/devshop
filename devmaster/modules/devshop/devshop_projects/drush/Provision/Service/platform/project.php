<?php

/**
 * The site_project service class.
 */
class Provision_Service_platform_project extends Provision_Service {
  public $service = 'platform_project';

  /**
   * Add environment, project, and git ref to site aliases.
   */
  static function subscribe_platform($context) {
    $context->setProperty('environment');
    $context->setProperty('project');
    $context->setProperty('git_ref');
  }
}
