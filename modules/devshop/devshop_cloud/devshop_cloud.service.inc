<?php

/**
 * The base service type class , registered with hook_hosting_service_type.
 */
class hostingService_provider extends hostingService {
  /**
   * the value stored in the service column of hosting_service table.
   */
  public $service = 'provider';
  public $provider_options = array();

  public $server_node = NULL;

  function __construct($node, $values = null) {
    parent::__construct($node, $values);
    $this->server_node = $node;
  }
}

/**
 * DigitalOcean provider.
 */
class hostingService_provider_digitalocean extends hostingService_provider {
  public $type = 'digitalocean';
}

/**
 * rackspace provider.
 */
class hostingService_provider_rackspace extends hostingService_provider {
  public $type = 'rackspace';
}

/**
 * softlayer provider.
 */
class hostingService_provider_softlayer extends hostingService_provider {
  public $type = 'softlayer';

  /**
   * Constructor: Sets provider options.
   * @param $node
   * @param null $values1
   */
  function __construct($node, $values = null) {
    parent::__construct($node, $values);

    $this->provider_options = array(
      'hostname' => $this->server->title,
      'domain' => $this->server->title,
      'processorCoreAmount' => 1,
      'memoryCapacity' => 1,
      'hourlyBillingFlag' => TRUE,
      'operatingSystemReferenceCode' => '',
    );
  }
}