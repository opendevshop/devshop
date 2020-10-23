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
   * A link to this server in the provider's dashboard.
   */
  public $provider_server_url = 'http://provider/server/%id%';

  public $subdomain;
  public $basedomain;
  public $ipAddress;
  public $serverNode;

  /**
   * Load ipAddress, base domain, subdomain into the class.
   *
   * hostingService_provider constructor.
   * @param $node
   * @param null $values
   */
  function __construct($node, $values = NULL)
  {
    parent::__construct($node, $values);
    $this->serverNode = $node;

    if (isset($this->getServer()->ipAddresses)) {
      $this->ipAddress = current($this->getServer()->ipAddresses);
    }

    $url_parts = explode('.', $this->getServer()->title);
    $this->basedomain = implode('.', array_slice($url_parts, -2, 2));
    $this->subdomain = str_replace('.' . $this->basedomain, '', $this->getServer()->title);
  }

  public function save($node) {
    parent::save($node);
    if (isset($node->nid)) {
      $this->serverNode = $node;
    }
  }

  public function getServer() {
    return $this->serverNode;
  }

  public function getProviderServerUrl() {
    return strtr($this->provider_server_url, array(
      '%id%' => $this->provider_server_identifier,
    ));
  }

  function form(&$form) {
    $form['provider_data'] = array(
      '#type' => 'value',
      '#value'=> array(),
    );
    $form['provider_server_identifier'] = array(
      '#type' => 'value',
      '#value'=> array(),
    );
  }
  /**
   * On server insert, we only have the provider_options.
   */
  function insert() {

    // Prevent duplicate insert... Not sure why this happens.
    if (!db_query('SELECT nid FROM {hosting_aegir_cloud_provider} WHERE nid = :nid', array(':nid' => $this->server->nid))->fetchField()) {
      db_insert('hosting_aegir_cloud_provider')
        ->fields(array(
          'nid',
          'provider_data',
          'provider_options',
          'provider_server_identifier'
        ), array(
          $this->server->nid,
          serialize($this->provider_data),
          serialize($this->provider_options),
          $this->provider_server_identifier
        ))
        ->execute();
    }
    parent::insert();
  }

  function delete() {
    parent::delete();
    db_query('DELETE FROM {hosting_aegir_cloud_provider} WHERE nid = :nid', array(':nid' => $this->server->nid));
  }

  function load() {
    parent::load();
    $this->mergeData('SELECT * FROM {hosting_aegir_cloud_provider} WHERE nid = :nid', array(':nid' => $this->server->nid));
    $this->provider_options = unserialize($this->provider_options);
    $this->provider_data = unserialize($this->provider_data);
  }
}


/**
 * rackspace provider.
 */
class hostingService_provider_rackspace extends hostingService_provider {
  public $type = 'rackspace';
}
