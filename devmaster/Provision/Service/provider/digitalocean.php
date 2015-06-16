<?php

use DigitalOceanV2\Adapter\BuzzAdapter;
use DigitalOceanV2\DigitalOceanV2;


class Provision_Service_provider_digital_ocean extends Provision_Service_provider {

  public $provider = 'digital_ocean';


  function verify_server_cmd() {

    $digitalocean = $this->load_api();
    $droplet = $digitalocean->droplet();
    $cloud = $droplet->getById($this->server->provider_server_identifier);
    if ($cloud->status == 'active') {

      $ips = array();
      foreach ($cloud->networks as $network) {
        $ips[] = $network->ipAddress;
      }

      drush_set_option('ip_addresses', $ips);
      drush_log('[DEVSHOP] Cloud Server IPs updated.', 'ok');
    }
    else {
      drush_set_error('DEVSHOP_CLOUD_SERVER_NOT_ACTIVE', dt('The remote cloud server is not in an active state.'));
    }
  }


  /**
   * This method is called once per server verify, triggered from hosting task.
   *
   * $this->server: Provision_Context_server
   */
  function save_server() {
    // Look for provider_server_identifier
    $server_identifier = $this->server->provider_server_identifier;

    // If server ID is already found, move on.
    if (!empty($server_identifier)) {
      drush_log('[DEVSHOP] Server Identifier Found: ' . $server_identifier . '  Not creating new server.', 'ok');
    }
    // If there is no server ID, create the server.
    else {
      drush_log('[DEVSHOP] Server Identifier not found.  Creating new server!', 'ok');

      $options = $this->server->provider_options;
      $digitalocean = $this->load_api();
      $droplet = $digitalocean->droplet();
      $created = $droplet->create($this->server->remote_host, $options['region'], $options['size'], $options['image'], array_filter($options['keys']));
      $this->server->setProperty('provider_server_identifier', $created->id);
      drush_log("[DEVSHOP] Server Identifier found: $created->id. Assumed server was created.", 'ok');
    }
  }

  function load_api(){
    $token = drush_get_option('digital_ocean_token');
    require_once dirname(__FILE__) . '/digital-ocean-master/vendor/autoload.php';
    require_once dirname(__FILE__) . '/digital-ocean-master/src/DigitalOceanV2.php';

    $adapter = new BuzzAdapter($token);
    $digitalocean = new DigitalOceanV2($adapter);
    return $digitalocean;
  }
}