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
    $context->setProperty('provider_data');
  }

  /**
   * Stub for init_server();
   *
   * Call from child classes:
   *   parent::init_server();
   *
   * This function is called many times during a server verify.
   * Use sparingly.
   */
  function init_server() {
  }

  /**
   * This method is called once per server verify.
   */
  function save_server() {
    // Save digitalocean specific info to the drush alias.
    $this->server->setProperty('ip_addresses');

  }

  static function option_documentation() {
    return array(
      '--provider' => 'The provider of this server. Must match an available Provision_Service_provider',
      '--provider_options' => 'An array of options to send to the provider.',
    );
  }
}
