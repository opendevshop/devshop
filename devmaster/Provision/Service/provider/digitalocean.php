<?php

class Provision_Service_provider_digitalocean extends Provision_Service_provider {

  protected $provider = 'digitalocean';

  /**
   * This method is called once per server verify, triggered from hosting task.
   *
   * $this->server: Provision_Context_server
   */
  function save_server() {

    // Look for provider_server_identifier
    $server_identifier = $this->server->provider_server_identifier;

    // If server ID is already found, move on.
    if ($server_identifier) {
      drush_log('[DEVSHOP] Server Identifier Found.  Not creating new server.', 'ok');
    }
    // If there is no sever ID, create the server.
    else {

      drush_log('[DEVSHOP] Server Identifier not found.  Creating new server!', 'ok');

      // Faking our provider_data response.
      $this->server->setProperty('provider_data', array(
        'hello' => 'do',
        'fake data' => 'from digitalocean',
      ));

      // Faking our provider server identifier.
      $this->server->setProperty('provider_server_identifier', '123456789');

      $this->server->setProperty('ip_addresses', array(
        '1.2.3.4'
      ));
    }
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
}