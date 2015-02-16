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
    // This is the wrong way.  Add insert/update methods.

//    // Break up title into hostname (subdomain) and domain.
//    $domain = explode('.', $node->title);
//
//    $this->provider_options = new stdClass();
//    $this->provider_options->hostname = $domain[0];
//    $this->provider_options->domain = implode('.', array_slice($domain, 1));
//    $this->provider_options->startCpus = $values['processors'];
//    $this->provider_options->maxMemory = $values['memory'];
//    $this->provider_options->hourlyBillingFlag = TRUE;
//    $this->provider_options->localDiskFlag = TRUE;
//    $this->provider_options->dedicatedAccountHostOnlyFlag = FALSE;
//    $this->provider_options->operatingSystemReferenceCode =  $values['operatingSystems'];
//
//    $this->provider_options->datacenter = new stdClass();
//    $this->provider_options->datacenter->name = $values['datacenter'];
//    $this->provider_options->api_key = variable_get('devshop_cloud_softlayer_api_key','YOURKEY');
//    $this->provider_options->api_username = variable_get('devshop_cloud_softlayer_api_username', 'YOURUSERNAME');
  }

  function form(&$form) {
    parent::form($form);

    $options = $this->getOptions();
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
  }

  /**
   * Process options
   */
  function getOptions() {
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
}