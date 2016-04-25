<?php

function devshop_cloud_settings_form() {
  $form = array();
  $form['note'] = array(
    '#value' => t('You must enable at least one cloud provider module.'),
    '#prefix' => '<div>',
    '#suffix' => '</div>',
  );
  $devshop_key = variable_get('devshop_public_key');
  $form['devshop_cloud_ssh_key'] = array(
    '#title' => t('DevShop Cloud Public SSH Key'),
    '#description' => t('Enter the public key of the user that will be connecting and provisioning your servers. This SSH key will be used to grant access to root on new servers created by DevShop Cloud.'),
    '#default_value' => variable_get('devshop_cloud_ssh_key', $devshop_key),
    '#type' => 'textarea',
  );
  return system_settings_form($form);
}