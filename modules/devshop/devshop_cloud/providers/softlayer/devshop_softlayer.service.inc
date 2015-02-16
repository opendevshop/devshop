<?php

module_load_include('service.inc', 'devshop_cloud');

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

    // Break up title into hostname (subdomain) and domain.
    $domain = explode('.', $node->title);

    $this->provider_options = new stdClass();
    $this->provider_options->hostname = $domain[0];
    $this->provider_options->domain = implode('.', array_slice($domain, 1));
    $this->provider_options->startCpus = 2;
    $this->provider_options->maxMemory = 4;
    $this->provider_options->hourlyBillingFlag = TRUE;
    $this->provider_options->localDiskFlag = TRUE;
    $this->provider_options->dedicatedAccountHostOnlyFlag = FALSE;
    $this->provider_options->operatingSystemReferenceCode = 'UBUNTU_12_64';

    $this->provider_options->datacenter = new stdClass();
    $this->provider_options->datacenter->name = 'par01';
    $this->provider_options->api_key = variable_get('devshop_cloud_softlayer_api_key','YOURKEY');
    $this->provider_options->api_username = variable_get('devshop_cloud_softlayer_api_username', 'YOURUSERNAME');
  }
}