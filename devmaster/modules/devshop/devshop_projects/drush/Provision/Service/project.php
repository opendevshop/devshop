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
    drush_log('Provision_Service_project::subscribe_site()', 'devshop_log');
    $context->setProperty('environment');
    $context->setProperty('project');
    $context->setProperty('git_ref');
  }

  /**
   * Add environment, project, and git ref to platform aliases.
   */
  static function subscribe_platform($context) {
    drush_log('Provision_Service_project::subscribe_platform()', 'devshop_log');
    $context->setProperty('environment');
    $context->setProperty('project');
    $context->setProperty('git_ref');
  }

}
