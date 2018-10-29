<?php

function aegir_cloud_settings_form() {
  $form = array();
  $form['note'] = array(
    '#value' => t('You must enable at least one cloud provider module.'),
    '#prefix' => '<div>',
    '#suffix' => '</div>',
  );
  $devshop_key = variable_get('aegir_public_key', '');
  $form['aegir_public_key'] = array(
    '#title' => t('Hostmaster Public SSH Key'),
    '#description' => t('Enter the public key of the user that will be connecting and provisioning your servers (typically "aegir". This SSH key will be used to grant access to root on new servers created by Aegir Cloud.'),
    '#default_value' => variable_get('aegir_public_key', $devshop_key),
    '#type' => 'textarea',
  );
  return system_settings_form($form);
}
