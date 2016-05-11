<?php

/**
 * Form function for the softlayer options form.
 * @return array
 */
function aegir_softlayer_options_form() {
  $form = array();

  $username = variable_get('aegir_cloud_softlayer_api_username', array());
  $key = variable_get('aegir_cloud_softlayer_api_key', array());

  if (empty($username) || empty($key)) {
    $form['warning'] = array(
      '#prefix' => '<div class="alert alert-danger">',
      '#suffix' => '</div>',
      '#value' => t('You must enter your softlayer username and API key before you can use this form.  See !link', array('!link' => l(t('Cloud Settings'), 'admin/hosting/aegir/cloud'))),
      '#weight' => 10,
    );
  }
  else {
    $options = variable_get('aegir_cloud_softlayer_options', array());

    $form['info'] = array(
      '#type' => 'item',
      '#title' => t('SoftLayer Options'),
      '#markup' => empty($options)? t('No options available. Click "Refresh SoftLayer Options".'): 'SoftLayer options are saved.',
    );

    $keys = variable_get('aegir_cloud_softlayer_ssh_keys', array());
    $key_count = count($keys);
    $form['keys'] = array(
      '#type' => 'item',
      '#title' => t('SoftLayer SSH Keys'),
      '#markup' => format_plural($key_count, t('One key available'), t('!num keys available', array(
      '!num' => $key_count
    ))),
    );

    // Get fingerprint of ssh key for comparison.
    $key_vars['key'] = (object) sshkey_parse(variable_get('aegir_cloud_public_key', ''));
    $fingerprint = theme_sshkey_fingerprint($key_vars);
    foreach ($keys as $key) {
      if ($fingerprint == $key->fingerprint) {
        $key_found = TRUE;
        drupal_set_message(t('!link key was found in your !link2. Ready to Provision.', array(
          '!link' => l(t('DevShop Cloud public key'), 'admin/aegir/cloud'),
          '!link2' => l(t('SoftLayer Account'), 'https://control.softlayer.com/devices/sshkeys'),
        )));
      }
    }

    if (!$key_found) {
      drupal_set_message(t('!link1 was not found in your SoftLayer Account.  !link2', array(
        '!link1' => l(t('DevShop Cloud public key'), 'admin/aegir/cloud'),
        '!link2' => l(t('Add your public key to Softlayer'), 'https://control.softlayer.com/devices/sshkeys'),
      )), 'error');
    }

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
function aegir_softlayer_options_form_submit() {

  require_once dirname(__FILE__) . '/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

  $apiUsername = variable_get('aegir_cloud_softlayer_api_username', array());
  $apiKey = variable_get('aegir_cloud_softlayer_api_key', array());

  // Get Create options
  try {
    $client = SoftLayer_SoapClient::getClient('SoftLayer_Virtual_Guest', null, $apiUsername, $apiKey);
    $options['options'] = $client->getCreateObjectOptions();
    variable_set('aegir_cloud_softlayer_options', $options['options']);

    $ssh_key_client  = SoftLayer_SoapClient::getClient('SoftLayer_Account', null, $apiUsername, $apiKey);
    $ssh_keys = $ssh_key_client->getSshKeys();

    variable_set('aegir_cloud_softlayer_ssh_keys', $ssh_keys);

    // Save a variable with an array ready to go for form options.
    $key_vars['key'] = (object) sshkey_parse(variable_get('aegir_cloud_public_key', ''));
    $fingerprint = theme_sshkey_fingerprint($key_vars);
    foreach ($ssh_keys as $key) {
      $ssh_key_options[$key->id] = $key->label;

      // Save the softlayer key ID for this aegir_cloud_public_key.
      if ($fingerprint == $key->fingerprint) {
        variable_set('aegir_cloud_public_key_softlayer_id', $key->id);
      }
    }

    variable_set('aegir_cloud_softlayer_ssh_keys_options', $ssh_key_options);

    drupal_set_message(t('SoftLayer options and SSH keys have been retrieved.'));

  } catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
  }
}