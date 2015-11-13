<?php

/**
 * The site_project service class.
 */
class Provision_Service_site_project extends Provision_Service {
  public $service = 'site_project';

  /**
   * Add environment, project, and git ref to site aliases.
   */
  static function subscribe_site($context) {
    $context->setProperty('environment');
    $context->setProperty('project');
    $context->setProperty('git_ref');
  }
}
