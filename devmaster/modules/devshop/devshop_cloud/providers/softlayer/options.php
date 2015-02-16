<?php

/**
 * Form function for the softlayer options form.
 * @return array
 */
function devshop_softlayer_options_form() {
  $form = array();

  $options = variable_get('devshop_cloud_softlayer_options', array());

  $form['info'] = array(
    '#type' => 'item',
    '#title' => t('SoftLayer Options'),
    '#value' => empty($options)? t('No options available. Click "Refresh SoftLayer Options".'): print_r($options, 1),
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
  return $form;
}

/**
 *
 */
function devshop_softlayer_options_form_submit() {

  drupal_set_message('Attempting to get options.');

}