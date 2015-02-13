<?php

/**
 * The Provision_Service_provider service class.
 */
class Provision_Service_provider extends Provision_Service {
  public $service = 'provider';

  /**
   * Add provider and provider options to the server drush alias.
   */
  static function subscribe_server($context) {
    $context->setProperty('provider');
    $context->setProperty('provider_options');
  }

  function verify_server_cmd() {
    drush_log('[DEVSHOP] Verifying Server...' . d()->remote_host, 'ok');
  }

  static function option_documentation() {
    return array(
      '--provider' => 'The provider of this server. Must match an available Provision_Service_provider',
      '--provider_options' => 'An array of options to send to the provider.',
    );
  }
}
