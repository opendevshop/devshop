<?php

/**
 * Digital Ocean provider.
 */
class hostingService_provider_digital_ocean extends hostingService_provider {
  public $type = 'digital_ocean';
  public $name = 'DigitalOcean';

  public $provider_server_url = 'https://cloud.digitalocean.com/droplets/%id%';

  function load_api() {
    return aegir_digitalocean_load_api();
  }

  function form(&$form) {
    parent::form($form);

    // Don't allow editing.
    if (!$this->server->nid) {
      $default_options = variable_get('aegir_digital_ocean_default_options', array());
      $options = $this->getDigitalOceanOptions();

      // If there are no options, show a link to the settings form.
      if (empty($options)) {
        $form['warning'] = array(
            '#markup' => t('You must first configure your !link to use this provider.', array(
                    '!link' => l(t('DigitalOcean Credentials'), 'admin/hosting/cloud'))
            ),
        );
        return;
      }

      $form['provider_options'] = array();
      $form['provider_options']['region'] = array(
        '#type' => 'select',
        '#title' => t('Region'),
        '#options' => $options['regions'],
        '#default_value' => $default_options['aegir_digital_ocean_region_default'],
      );
      $form['provider_options']['image'] = array(
        '#type' => 'select',
        '#title' => t('Image'),
        '#options' => $options['images'],
        '#default_value' => $default_options['aegir_digital_ocean_default_image'],
      );
      $form['provider_options']['size'] = array(
        '#type' => 'select',
        '#title' => t('Server Size'),
        '#options' => $options['sizes'],
        '#default_value' => $default_options['aegir_digital_ocean_default_size'],
      );
      $form['provider_options']['keys'] = array(
        '#type' => 'checkboxes',
        '#title' => t('SSH Keys'),
        '#options' => $options['keys'],
        '#default_value' => $default_options['aegir_digital_ocean_default_keys'],
      );
      $form['provider_options']['backups'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable Backups'),
        '#default_value' => $default_options['aegir_digital_ocean_default_backups'],
      );
      $form['provider_options']['ipv6'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable IPv6'),
        '#default_value' => $default_options['aegir_digital_ocean_default_ipv6'],
      );
      $form['provider_options']['private_networking'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable Private Networking'),
        '#default_value' => $default_options['aegir_digital_ocean_default_private_networking'],
      );
      $form['provider_options']['user_data'] = array(
        '#type' => 'textarea',
        '#title' => t('Cloud Init'),
        '#default_value' => $default_options['aegir_digital_ocean_default_user_data'],
        '#description' => t('Cloud Init (aka User Data) is a standardized way to pre-configure your server, supported by most cloud server providers. If this field is Bash, it is run as soon as the server is ready. If this field has YML, it can do many things. See !link for Cloud Init examples', array(
          '!link' => l('cloudinit.readthedocs.io', 'https://cloudinit.readthedocs.io/en/latest/topics/examples.html')
        )),
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
//      $api = $this->softlayer_client('SoftLayer_Account');
//      $account = $api->getObject();
//      $this->node = $node;

      // @TODO: Check API Access.

      // Detect form errors.
      // If there are errors, do not attempt to create server.
      $errors = form_get_errors();
      if ($errors) {
        drupal_set_message(t('Validation errors must be fixed before we can create the server.'), 'error');
      }
      // If there are no errors, create the server.
      // We do this in the validation phase so that we can cause form errors if something goes wrong.
      else {

        // Get DigitalOcean Droplet API.
        $api = $this->load_api();
        $droplet = $api->droplet();

        $keys = array_filter($this->provider_options['keys']);
        $keys = array_values($keys);

        $provider_data = $droplet->create($node->title,
            $this->provider_options['region'],
            $this->provider_options['size'],
            $this->provider_options['image'],
            $this->provider_options['backups'],
            $this->provider_options['ipv6'],
            $this->provider_options['private_networking'],
            $keys,
            $this->provider_options['user_data']
        );

        form_set_value($form['services']['provider']['digital_ocean']['provider_data'], $provider_data, $form_state);
        form_set_value($form['services']['provider']['digital_ocean']['provider_server_identifier'], $provider_data->id, $form_state);
      }

    } catch (Exception $e) {
      form_set_error('provider_options', $e->getMessage());
      drupal_set_message(t('Check the !link settings page and click "Refresh DigitalOcean Options', array(
        '!link' => l('Cloud', 'admin/hosting/cloud'),
      )), 'error');
    }
  }

  /**
   * Process options
   */
  function getDigitalOceanOptions() {
    $options = variable_get('aegir_cloud_digital_ocean_options', array());
    return $options;
  }

  public function context_options($task_type, $ref_type, &$task) {
    parent::context_options($task_type, $ref_type, $task);
    $task->context_options['digital_ocean_token'] = variable_get('aegir_cloud_digital_ocean_api_token', NULL);
  }

  /**
   * Return a list of all DNS entries that match either the domain or the IP address of this server.
   */
  function findDnsEntries() {
    $records = array();

    try {
      $domainRecords =  $this->load_api()->domainRecord()->getAll($this->basedomain);
      foreach ($domainRecords as $record) {
        if (strpos($record->name, $this->subdomain) !== FALSE || $record->data == $this->ipAddress) {
          $records[] = $record;
        }
      }
    } catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'warning');
    }
    return $records;
  }

  /**
   * Return a list ready for drupal items or options.
   */
  function dnsEntriesOptionsList() {
    $records = $this->findDnsEntries();

    foreach ($records as $record) {
      $items[$record->id] = "<code>$record->type $record->name.$this->basedomain $record->data</code> (ID: $record->id )";
    }
    return $items;
  }

  /**
   * Absolutely destroy this server using the packet API.
   */
  public function destroy() {
    try {
      $api = $this->load_api();
      $droplet = $api->droplet();
      $droplet->delete($this->provider_server_identifier);

      drupal_set_message(t('The server was destroyed.'));
    } catch(Exception $e) {
      form_set_error('provider', 'API Error: ' . $e->getMessage());
    }
  }

  /**
   * Destroy DNS records for this service.
   */
  public function destroyDnsRecords($records) {
    $records = array_filter($records);
    try {
      foreach ($records as $record_id) {
        $this->load_api()->domainRecord()->delete($this->basedomain, $record_id);
      }
    } catch(Exception $e) {
      form_set_error('provider', 'API Error: ' . $e->getMessage());
    }
  }
}
