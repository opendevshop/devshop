<?php

/**
 * The project service class.
 */
class Provision_Service_project extends Provision_Service {
  public $service = 'project';

  /**
   * Add environment, project, and git ref to site aliases.
   */
  static function subscribe_site($context) {
    $context->setProperty('environment');
    $context->setProperty('project');
    $context->setProperty('git_ref');
  }

  /**
   * Add environment, project, and git ref to platform aliases.
   */
  static function subscribe_platform($context) {
    $context->setProperty('environment');
    $context->setProperty('project');
    $context->setProperty('git_ref');
  }

}
