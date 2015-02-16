<?php

module_load_include('service.inc', 'devshop_cloud');

/**
 * softlayer provider.
 */
class hostingService_provider_softlayer extends hostingService_provider {
  public $type = 'softlayer';

  function form(&$form) {
    parent::form($form);

    if (empty($this->provider_server_identifier)){
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

  public function context_options($task_type, $ref_type, &$task) {
    $task->context_options['softlayer_api_username'] = variable_get('devshop_cloud_softlayer_api_username','YOURUSERNAME');
    $task->context_options['softlayer_api_key'] = variable_get('devshop_cloud_softlayer_api_key','YOURKEY');
  }
}