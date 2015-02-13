<?php

/**
 * The site_project service class.
 */
class Provision_Service_server_provider extends Provision_Service {
  public $service = 'provider';

  /**
   * Add environment, project, and git ref to site aliases.
   */
  static function subscribe_server($context) {
    $context->setProperty('provider');
    $context->setProperty('provider_options');
  }

  function verify() {
    $this->type_invoke('verify');
    drush_log('[DEVSHOP] verify()', 'ok');
  }

  function verify_server_cmd() {
    $this->create_config($this->context->type);
    $this->parse_configs();
  }
}
