<?php
require_once dirname(__FILE__) . '/provider/digitalocean.php';
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
    $context->setProperty('provider_server_identifier');
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
    parent::init();
  }

  /**
   * Saves server options to drush options so they will be picked up by
   * devshop_cloud_post_hosting_verify_task()
   */
  function verify_server_cmd() {
    drush_set_option('provider_data', $this->server->provider_data);
    drush_set_option('provider_server_identifier', $this->server->provider_server_identifier);

    if (!empty($this->server->ip_addresses)) {
      drush_set_option('ip_addresses', $this->server->ip_addresses);
    }
  }

  static function option_documentation() {
    return array(
      '--provider' => 'The provider of this server. Must match an available Provision_Service_provider',
      '--provider_options' => 'An array of options to send to the provider.',
    );
  }
}
