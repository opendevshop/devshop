<?php

class Provision_Service_provider_softlayer extends Provision_Service_provider {

  protected $provider = 'softlayer';

  /**
   * This method is called once per server verify.
   */
  function save_server() {
    drush_log('[DEVSHOP|softlayer] Provision_Service_provider_softlayer->save_server()', 'ok');

    // Save digitalocean specific info to the drush alias.
    $this->server->setProperty('softlayer_thing', 'FAKEDROPID');

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
//    drush_log('[DEVSHOP|softlayer] ' .print_r($provider_options, 1), 'notice');

    require_once dirname(__FILE__) . '/SoftLayer/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

    $apiUsername = drush_get_option('softlayer-api-username', '');
    $apiKey = drush_get_option('softlayer-api-key', '');


    // Initialize an API client for the SoftLayer_Account service.
    $hardware = SoftLayer_SoapClient::getClient('SoftLayer_Hardware', null, $apiUsername, $apiKey);

    // Retrieve our account record
    try {
      $server = $hardware->createObject($provider_options);
      print_r($server);

    } catch (Exception $e) {

      return drush_set_error('DEVSHOP_CLOUD_API_ACCESS_DENIED', $e->getMessage());
    }
  }
}