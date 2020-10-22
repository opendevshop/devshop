<?php


class Provision_Service_provider_softlayer extends Provision_Service_provider {

  protected $provider = 'softlayer';

  /**
   * This method is called once per server verify.
   */
  function save_server() {
    // Look for provider_server_identifier
    $server_identifier = $this->server->provider_server_identifier;

    // If server ID is already found, move on.
    if ($server_identifier) {
      drush_log('[DEVSHOP] Server Identifier Found.  Not creating new server.', 'ok');
    }
    // If there is no server ID, create the server.
    else {

      drush_log('[DEVSHOP] Server Identifier not found.  Creating new server!', 'ok');

      $server_fqdn = d()->remote_host;

      drush_log('[DEVSHOP|softlayer] Creating Server ' . $server_fqdn . '...', 'notice');

      // Initialize an API client for the SoftLayer_Account service.
      $virtual_guest = $this->softlayer_client('SoftLayer_Virtual_Guest');
      $provider_options = $this->prepare_provider_options();

      // Retrieve our account record
      try {

        // @TODO: Add more robust simulation.
        //$server = array(
        //  'id' => '00000',
        //  'stuff' => 'from softlayer',
        //);
        $server = (array) $virtual_guest->createObject($provider_options);

        drush_log('[DEVSHOP|softlayer] Created server in softlayer: ' . $server['id'], 'ok');
      } catch (Exception $e) {
        return drush_set_error('DEVSHOP_CLOUD_API_ACCESS_DENIED', $e->getMessage());
      }

      $provider_data = (array) $server;
      $this->server->setProperty('provider_data', $provider_data);
      $this->server->setProperty('provider_server_identifier', $provider_data['id']);
    }
  }

  function prepare_provider_options() {
    $devshop_cloud_provider_options = (object) drush_get_option('provider_options', '');

    // Break up title into hostname (subdomain) and domain.
    $provider_options = new stdClass();
    $domain = explode('.', d()->remote_host);
    $provider_options->hostname = $domain[0];
    $provider_options->domain = implode('.', array_slice($domain, 1));
    $provider_options->startCpus = $devshop_cloud_provider_options->processors;
    $provider_options->maxMemory = $devshop_cloud_provider_options->memory;
    $provider_options->hourlyBillingFlag = TRUE;
    $provider_options->localDiskFlag = TRUE;
    $provider_options->dedicatedAccountHostOnlyFlag = FALSE;
    $provider_options->operatingSystemReferenceCode = $devshop_cloud_provider_options->operatingSystems;

    $provider_options->datacenter = new stdClass();
    $provider_options->datacenter->name = $devshop_cloud_provider_options->datacenter;

    return $provider_options;
  }

  /**
   * Helper for getting a softlayer client.
   * @param $service
   * @return \Softlayer_SoapClient
   */
  private function softlayer_client($service, $id = null) {
    $api_key = drush_get_option('softlayer_api_key');
    $username = drush_get_option('softlayer_api_username');

    // Initialize an API client for the SoftLayer_Account service.
    require_once dirname(__FILE__) . '/SoftLayer/softlayer-api-php-client/SoftLayer/SoapClient.class.php';
    $client = SoftLayer_SoapClient::getClient($service, $id, $username, $api_key);
    return $client;
  }
}