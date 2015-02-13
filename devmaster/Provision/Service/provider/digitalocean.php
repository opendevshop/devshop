<?php

class Provision_Service_provider_digitalocean extends Provision_Service_provider {

  protected $provider = 'digitalocean';

  /**
   * This method is called once per server verify.
   */
  function save_server() {
    drush_log('[DEVSHOP|digitalocean] Provision_Service_provider_digitalocean->save_server()', 'ok');

    // Save digitalocean specific info to the drush alias.
    $this->server->setProperty('drop_id', 'FAKEDROPID');

    // $this->server->shell_exec($path . ' -V');

    // Call Provision_Service_provider::save_server() to do things like save the IP address.
    parent::save_server();

  }
}