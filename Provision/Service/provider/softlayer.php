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
}