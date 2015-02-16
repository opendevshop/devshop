<?php

/**
 * Form function for the softlayer options form.
 * @return array
 */
function devshop_softlayer_options_form() {
  $form = array();

  $username = variable_get('devshop_cloud_softlayer_api_username', array());
  $key = variable_get('devshop_cloud_softlayer_api_key', array());

  if (empty($username) || empty($key)) {
    $form['warning'] = array(
      '#prefix' => '<div class="alert alert-danger">',
      '#suffix' => '</div>',
      '#value' => t('You must enter your softlayer username and API key before you can use this form.  See !link', array('!link' => l(t('Cloud Settings'), 'admin/hosting/devshop/cloud'))),
      '#weight' => 10,
    );
  }
  else {
    $options = variable_get('devshop_cloud_softlayer_options', array());

    $form['info'] = array(
      '#type' => 'item',
      '#title' => t('SoftLayer Options'),
      '#value' => empty($options)? t('No options available. Click "Refresh SoftLayer Options".'): 'SoftLayer options are saved.',
    );

    $form['note'] = array(
      '#prefix' => '<div>',
      '#suffix' => '</div>',
      '#value' => t('Use the button below to retrieve available options from SoftLayer.'),
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Refresh SoftLayer Options'),
    );
  }

  return $form;
}

/**
 *
 */
function devshop_softlayer_options_form_submit() {

  require_once dirname(__FILE__) . '/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

  $apiUsername = variable_get('devshop_cloud_softlayer_api_username', array());
  $apiKey = variable_get('devshop_cloud_softlayer_api_key', array());

  // Get Create options
  try {
    $client = SoftLayer_SoapClient::getClient('SoftLayer_Virtual_Guest', null, $apiUsername, $apiKey);
    $options['options'] = $client->getCreateObjectOptions();
    variable_set('devshop_cloud_softlayer_options', $options['options']);
    drupal_set_message(t('SoftLayer options have been saved.'));

  } catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
  }
}