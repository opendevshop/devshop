<?php

require_once dirname(__FILE__) . '/SoftLayer/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

class Provision_Service_provider_softlayer extends Provision_Service_provider {

  protected $provider = 'softlayer';

  /**
   * This method is called once per server verify.
   */
  function save_server() {
    drush_log('[DEVSHOP|softlayer] Provision_Service_provider_softlayer->save_server()', 'ok');

    // Check for globalIdentifier
    $provider_data = d()->provider_data;
    if (empty($provider_data['globalIdentifier'])) {

      drush_log('[DEVSHOP|softlayer] No global identifier! Creating server...', 'ok');

      // If there is none, attempt to create a server
      $provider_data = (array) $this->create_softlayer_virtual_guest();
      $this->server->setProperty('provider_data', $provider_data);

      // @TODO: Wait for server using getObject()

    }
    else {
      drush_log('[DEVSHOP|softlayer] Global identifier found. skipping server creation.', 'ok');

    }

    // Call Provision_Service_provider::save_server() to do things like save the IP address.
    parent::save_server();
  }

  /**
   * Helper for getting a softlayer client.
   * @param $service
   * @return \Softlayer_SoapClient
   */
  private function softlayer_client($service, $id = null) {
    $provider_options = (object) drush_get_option('provider_options', '');

    // Initialize an API client for the SoftLayer_Account service.
    $client = SoftLayer_SoapClient::getClient($service, $id, $provider_options->api_username, $provider_options->api_key);
    return $client;
  }

  /**
   * Method to create a server.
   * @return mixed
   */
 function create_softlayer_virtual_guest() {
    $server_fqdn = d()->remote_host;
    $provider_options = (object) drush_get_option('provider_options', '');

    drush_log('[DEVSHOP|softlayer] Creating Server ' . $server_fqdn . '...', 'notice');

    // Initialize an API client for the SoftLayer_Account service.
    $virtual_guest = $this->softlayer_client('SoftLayer_Virtual_Guest');

    // Retrieve our account record
    try {
      $server = $virtual_guest->createObject($provider_options);
      drush_log('[DEVSHOP|softlayer] SoftLayer_Virtual_Guest::createObject() ' . print_r($server, 1), 'ok');
      return $server;
    } catch (Exception $e) {
      return drush_set_error('DEVSHOP_CLOUD_API_ACCESS_DENIED', $e->getMessage());
    }
  }
}