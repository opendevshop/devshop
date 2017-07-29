<?php

module_load_include('service.inc', 'aegir_cloud');


/**
 * packet provider.
 */
class hostingService_provider_packet extends hostingService_provider {
  public $type = 'packet';

  /**
   * The human readable name of this service.
   *
   * @var string
   */
  public $name = 'Packet.net';
  public $provider_server_url = 'https://app.packet.net/portal#/project/%project_id%/server/%server_id%/overview';

  public function getProviderServerUrl() {
    return strtr($this->provider_server_url, array(
      '%server_id%' => $this->provider_server_identifier,
      '%project_id%' => $this->provider_options['project'],
    ));
  }


  function form(&$form) {

    $options = variable_get('aegir_cloud_packet_options', array());
    if (empty($options)) {

      $form['warning'] = array(
        '#markup' => t('You must first configure your !link to use this provider.', array(
          '!link' => l(t('packet Credentials'), 'admin/hosting/cloud'))
        ),
      );
      return;
    }

    parent::form($form);

    // Don't allow editing.
    if (!$this->server->nid) {
      $options = variable_get('aegir_cloud_packet_options', array());

      $form['provider_options'] = array();
      $form['provider_options']['project'] = array(
        '#type' => 'select',
        '#title' => t('Project'),
        '#options' => $options['projects'],
      );
      $form['provider_options']['facility'] = array(
        '#type' => 'select',
        '#title' => t('Facility'),
        '#options' => $options['facilities'],
      );
      $form['provider_options']['os'] = array(
        '#type' => 'select',
        '#title' => t('Operating System'),
        '#options' => $options['oses'],
      );
      $form['provider_options']['plan'] = array(
        '#type' => 'select',
        '#title' => t('Type'),
        '#options' => $options['plans'],
      );
    }
  }

  /**
   * Validate the server form.
   * @param $node
   * @param $form
   */
  public function validate(&$node, &$form, &$form_state) {

    if (!empty($node->nid)) {
      return;
    }

    try {

      // Detect form errors.
      // If there are errors, do not attempt to create server.
      $errors = form_get_errors();
      if ($errors) {
        drupal_set_message(t('Validation errors must be fixed before we can create the server.'), 'error');
      }
      // If there are no errors, create the server.
      // We do this in the validation phase so that we can cause form errors if something goes wrong.
      else {

        // Create a device
        $service_data = $form_state['values']['services']['provider']['packet']['provider_options'];

        $device = new \PacketHost\Client\Domain\Device();
        $device->hostname = $node->title;
        $device->facility = $service_data['facility'];
        $device->plan = $service_data['plan'];
        $device->operatingSystem = $service_data['os'];
        $device->sshKeys = $service_data['ssh_keys'];

        $provider_data = $this->getApi()->device()->create($service_data['project'], $device);

        form_set_value($form['services']['provider']['packet']['provider_data'], $provider_data, $form_state);
        form_set_value($form['services']['provider']['packet']['provider_server_identifier'], $provider_data->id, $form_state);
      }
    } catch (Exception $e) {
      form_set_error('provider_options', $e->getMessage());
    }
  }

  /**
   * Server Node Insert.
   */
  function insert() {
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
    $domain = explode('.', $this->node->title);
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

    foreach (array_filter($this->provider_options['ssh_keys']) as $key_id) {
      $key = new stdClass();
      $key->id = $key_id;
      $provider_options->sshKeys[] = $key;
    }

    return $provider_options;
  }

  public function context_options($task_type, $ref_type, &$task) {
    parent::context_options($task_type, $ref_type, $task);
    $task->context_options['packet_api_key'] = variable_get('aegir_cloud_packet_api_key','YOURKEY');
  }

  /**
   * Absolutely destroy this server using the packet API.
   */
  public function destroy() {
    try {
      $this->getApi()->device()->delete($this->provider_options['project'], $this->provider_server_identifier);
      drupal_set_message(t('The Packet.net server was destroyed.'));
    } catch(Exception $e) {
      form_set_error('provider', 'API Error: ' . $e->getMessage());
    }
  }

  /**
   * Returns an active PacketAPI object
   *
   * @return \PacketHost\Client\PacketApi
   */
  function getApi() {
    $config = new PacketHost\Client\Adapter\Configuration\DefaultConfiguration(
      variable_get('aegir_cloud_packet_api_key', '')
    );
    $adapter = new PacketHost\Client\Adapter\GuzzleAdapter($config);
    $api = new PacketHost\Client\PacketApi($adapter);

    return $api;
  }
}