<?php

module_load_include('service.inc', 'devshop_cloud');

require_once dirname(__FILE__) . '/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

/**
 * softlayer provider.
 */
class hostingService_provider_softlayer extends hostingService_provider {
  public $type = 'softlayer';

  function form(&$form) {
    parent::form($form);

    // Don't allow editing.
    if (!$this->server->nid) {
      $options = $this->getSoftlayerOptions();
      $form['provider_options'] = array();
      $form['provider_options']['datacenter'] = array(
        '#type' => 'select',
        '#title' => t('Datacenter'),
        '#options' => $options['datacenters'],
      );
      $form['provider_options']['memory'] = array(
        '#type' => 'select',
        '#title' => t('Memory'),
        '#options' => $options['memory'],
      );
      $form['provider_options']['operatingSystems'] = array(
        '#type' => 'select',
        '#title' => t('Operating System'),
        '#options' => $options['operatingSystems'],
      );
      $form['provider_options']['processors'] = array(
        '#type' => 'select',
        '#title' => t('Processors'),
        '#options' => $options['processors'],
      );
      $form['provider_options']['ssh_keys'] = array(
        '#type' => 'checkboxes',
        '#title' => t('SSH Keys'),
        '#options' => variable_get('devshop_cloud_softlayer_ssh_keys_options', array()),
        '#default_value' => array(
          variable_get('devshop_cloud_public_key_softlayer_id', ''),
        ),
      );
    }
  }

  /**
   * Validate the server form.
   * @param $node
   * @param $form
   */
  public function validate(&$node, &$form) {
    try {
      $api = $this->softlayer_client('SoftLayer_Account');
      $account = $api->getObject();
      drupal_set_message('Softlayer API connection was successful.');
      $this->node = $node;
    } catch (Exception $e) {
      form_set_error('provider_options', $e->getMessage());
    }
  }

  /**
   * Server Node Insert.
   */
  function insert() {
    try {

      // Create the Softlayer "Virtual Guest"
      $api = $this->softlayer_client('SoftLayer_Virtual_Guest');
      $this->provider_data = $api->createObject($this->prepare_server_options());
      $this->provider_server_identifier = $this->provider_data->id;

    } catch (Exception $e) {
      form_set_error('provider_options', $e->getMessage());
    }

    parent::insert();
  }

  /**
   * Prepares this server's options.
   *
   * Use the "SoftLayer_Virtual_Guest" datatype reference:
   * http://sldn.softlayer.com/reference/datatypes/SoftLayer_Virtual_Guest
   *
   * @return \stdClass
   */
  function prepare_server_options() {

    // Break up title into hostname (subdomain) and domain.
    $provider_options = new stdClass();
    $domain = explode('.', $this->server->title);
    $provider_options->hostname = $domain[0];
    $provider_options->domain = implode('.', array_slice($domain, 1));
    $provider_options->startCpus = $this->provider_options['processors'];
    $provider_options->maxMemory = $this->provider_options['memory'];
    $provider_options->hourlyBillingFlag = TRUE;
    $provider_options->localDiskFlag = TRUE;
    $provider_options->dedicatedAccountHostOnlyFlag = FALSE;
    $provider_options->operatingSystemReferenceCode = $this->provider_options['operatingSystems'];

    $provider_options->datacenter = new stdClass();
    $provider_options->datacenter->name = $this->provider_options['datacenter'];

    return $provider_options;
  }

  /**
   * Process options
   */
  function getSoftlayerOptions() {
    $options = array();
    $raw_options = variable_get('devshop_cloud_softlayer_options', array());

    if (empty($raw_options)) {
      drupal_set_message(t('You must !link before you can create a server there.', array('!link' => l(t('retrieve SoftLayer options'), 'admin/hosting/devshop/softlayer'))), 'error');
      return;
    }

    // Datacenters
    foreach ($raw_options->datacenters as $object) {
      $options['datacenters'][$object->template->datacenter->name] = $object->template->datacenter->name;
    }
    ksort($options['datacenters']);

    // Memory
    foreach ($raw_options->memory as $object) {
      $options['memory'][$object->template->maxMemory] = $object->itemPrice->item->description;
    }

    // OS
    foreach ($raw_options->operatingSystems as $object) {
      $options['operatingSystems'][$object->template->operatingSystemReferenceCode] = $object->itemPrice->item->description;
    }

    // CPUs
    foreach ($raw_options->processors as $object) {
      $options['processors'][$object->template->startCpus] = $object->itemPrice->item->description;
    }

    return $options;
  }

  /**
   * Helper for getting a softlayer client.
   * @param $service
   * @return \Softlayer_SoapClient
   */
  private function softlayer_client($service, $id = null) {
    $api_key = variable_get('devshop_cloud_softlayer_api_key');
    $username = variable_get('devshop_cloud_softlayer_api_username');

    // Initialize an API client for the SoftLayer_Account service.
    $client = SoftLayer_SoapClient::getClient($service, $id, $username, $api_key);
    return $client;
  }

  public function context_options($task_type, $ref_type, &$task) {
    parent::context_options($task_type, $ref_type, $task);
    $task->context_options['softlayer_api_username'] = variable_get('devshop_cloud_softlayer_api_username','YOURUSERNAME');
    $task->context_options['softlayer_api_key'] = variable_get('devshop_cloud_softlayer_api_key','YOURKEY');
  }
}