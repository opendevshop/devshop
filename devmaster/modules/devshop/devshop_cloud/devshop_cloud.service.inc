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

  public $server_node = NULL;

  function __construct($node, $values = null) {
    parent::__construct($node, $values);
    $this->server_node = $node;
    $this->provider_data = variable_get("devshop_cloud_server_data_{$node->nid}", '');
    $this->provider_server_identifier = variable_get("devshop_cloud_server_id_{$node->nid}", '');
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
