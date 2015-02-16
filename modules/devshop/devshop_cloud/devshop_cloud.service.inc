<?php

/**
 * The base service type class , registered with hook_hosting_service_type.
 */
class hostingService_provider extends hostingService {
  /**
   * the value stored in the service column of hosting_service table.
   */
  public $service = 'provider';

  /**
   * Options used to create the server.
   */
  public $provider_options = array();

  /**
   * Data returned back from provider about the server.
   */
  public $provider_data = array();

  /**
   * This server's unique identifier in the provider's system.
   */
  public $provider_server_identifier = '';

  /**
   * On server insert, we only have the provider_options.
   */
  function insert() {
    parent::insert();
    db_query("INSERT INTO {hosting_devshop_cloud_provider} (nid, provider_options, provider_data, provider_server_identifier)
      VALUES (%d, '%s', '%s', '%s')",
      $this->server->nid, serialize($this->provider_options), '', '');
  }

  /**
   * On server update, we only want to save provider data and provider_server_identifier.
   */
  function update() {
    parent::update();
    if ($this->provider_server_identifier) {
      db_query("UPDATE {hosting_devshop_cloud_provider} SET provider_data = '%s', provider_server_identifier = '%s' WHERE nid = %d", serialize($this->provider_data), $this->provider_server_identifier, $this->server->nid);
    }
  }

  function delete() {
    parent::delete();
    db_query('DELETE FROM {hosting_devshop_cloud_provider} WHERE nid = %d', $this->server->nid);
  }

  function load() {
    parent::load();
    $this->mergeData('SELECT * FROM {hosting_devshop_cloud_provider} WHERE nid = %d', $this->server->nid);
    $this->provider_options = unserialize($this->provider_options);
    $this->provider_data = unserialize($this->provider_data);
  }
}

/**
 * DigitalOcean provider.
 */
class hostingService_provider_digitalocean extends hostingService_provider {
  public $type = 'digitalocean';

  /**
   * Constructor: Sets provider options.
   * @param $node
   * @param null $values1
   */
  function __construct($node, $values = null) {
    parent::__construct($node, $values);

    $this->provider_options = new stdClass();
    $this->provider_options->hostname = $node->title;
    $this->provider_options->size = '2GB';

  }

}

/**
 * rackspace provider.
 */
class hostingService_provider_rackspace extends hostingService_provider {
  public $type = 'rackspace';
}
