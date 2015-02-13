<?php

class Provision_Service_provider_softlayer extends Provision_Service_provider {

  protected $provider = 'softlayer';

  /**
   * This method is called once per server verify.
   */
  function save_server() {
    drush_log('[DEVSHOP|softlayer] Provision_Service_provider_softlayer->save_server()', 'ok');

    // Example property.
    $this->server->setProperty('devshop_cloud', TRUE);

    // $this->server->shell_exec($path . ' -V');

    // Call Provision_Service_provider::save_server() to do things like save the IP address.
    parent::save_server();
  }

  /**
   * Implements verify_server_cmd()
   */
  function verify_server_cmd(){

    $server_fqdn = d()->remote_host;
    $provider_options = (object) drush_get_option('provider_options', '');

    drush_log('[DEVSHOP|softlayer] Verifying Server ' . $server_fqdn . '...', 'notice');

    require_once dirname(__FILE__) . '/SoftLayer/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

    // Initialize an API client for the SoftLayer_Account service.
    $hardware = SoftLayer_SoapClient::getClient('SoftLayer_Hardware', null, $provider_options->api_username, $provider_options->api_key);

    drush_log('[DEVSHOP|softlayer] API: ' . $provider_options->api_username . '...' . $provider_options->api_key, 'notice');

    // Retrieve our account record
    try {
      $server = $hardware->createObject($provider_options);
      drush_log(print_r($server, 1), 'ok');

      // SERVER!?

    } catch (Exception $e) {

      return drush_set_error('DEVSHOP_CLOUD_API_ACCESS_DENIED', $e->getMessage());
    }
  }
}