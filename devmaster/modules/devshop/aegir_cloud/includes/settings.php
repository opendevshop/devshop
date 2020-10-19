<?php

function aegir_cloud_settings_form() {
  $form = array();
  $form['note'] = array(
    '#value' => t('You must enable at least one cloud provider module.'),
    '#prefix' => '<div>',
    '#suffix' => '</div>',
  );
  $form['devshop_public_key'] = array(
    '#title' => t('SSH Key for new Cloud Servers'),
    '#description' => t("Enter a public SSH key to add to new cloud server's root user. WARNING: Anyone who has the private key that matches will have root access to the new server."),
    '#default_value' => variable_get('devshop_public_key', ''),
    '#type' => 'textarea',
  );
  return system_settings_form($form);
}
