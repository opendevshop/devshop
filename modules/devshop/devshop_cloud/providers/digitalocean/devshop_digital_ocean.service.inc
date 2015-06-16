<?php

module_load_include('service.inc', 'devshop_cloud');

/**
 * Digital Ocean provider.
 */
class hostingService_provider_digital_ocean extends hostingService_provider {
  public $type = 'digital_ocean';

  function form(&$form) {
    parent::form($form);

    // Don't allow editing.
    if (!$this->server->nid) {
      $default_options = variable_get('devshop_digital_ocean_default_options', array());
      dsm($default_options);
      $options = $this->getDigitalOceanOptions();
      $form['provider_options'] = array();
      $form['provider_options']['region'] = array(
        '#type' => 'select',
        '#title' => t('Region'),
        '#options' => $options['regions'],
        '#default_value' => $default_options['devshop_digital_ocean_default_region'],
      );
      $form['provider_options']['image'] = array(
        '#type' => 'select',
        '#title' => t('Image'),
        '#options' => $options['images'],
        '#default_value' => $default_options['devshop_digital_ocean_default_image'],
      );
      $form['provider_options']['size'] = array(
        '#type' => 'select',
        '#title' => t('Server Size'),
        '#options' => $options['sizes'],
        '#default_value' => $default_options['devshop_digital_ocean_default_size'],
      );
      $form['provider_options']['keys'] = array(
        '#type' => 'checkboxes',
        '#title' => t('SSH Keys'),
        '#options' => $options['keys'],
        '#default_value' => $default_options['devshop_digital_ocean_default_keys'],
      );
      $form['provider_options']['backups'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable Backups'),
        '#default_value' => $default_options['devshop_digital_ocean_default_backups'],
      );
      $form['provider_options']['ipv6'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable IPv6'),
        '#default_value' => $default_options['devshop_digital_ocean_default_ipv6'],
      );
      $form['provider_options']['private_networking'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable Private Networking'),
        '#default_value' => $default_options['devshop_digital_ocean_default_private_networking'],
      );
      $form['provider_options']['remote_server'] = array(
        '#type' => 'checkbox',
        '#title' => t('Setup as remote aegir server'),
        '#default_value' => $default_options['devshop_digital_ocean_default_remote_server'],
      );
      //$form['provider_options']['cloud_config'] = array(
      //  '#type' => 'textarea',
      //  '#title' => t('Cloud Config'),
      //  '#default_value' => $default_options['devshop_digital_ocean_default_cloud_config'],
      //);
    }
  }

  /**
   * Process options
   */
  function getDigitalOceanOptions() {
    $options = variable_get('devshop_cloud_digital_ocean_options', array());
    if (empty($options)) {
      drupal_set_message(t('You must !link before you can create a server there.', array('!link' => l(t('retrieve Digital Ocean options'), 'admin/hosting/devshop/digital_ocean'))), 'error');
      return;
    }
    return $options;
  }

  public function context_options($task_type, $ref_type, &$task) {
    parent::context_options($task_type, $ref_type, $task);
    $task->context_options['digital_ocean_token'] = variable_get('devshop_cloud_digital_ocean_api_token', NULL);
  }
}