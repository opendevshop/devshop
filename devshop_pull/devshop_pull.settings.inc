<?php

/**
 * Page callback for the settings page.
 */
function devshop_pull_settings_page(){
  $output .= drupal_get_form('devshop_pull_settings_form');
  return $output;
}

/**
 * General settings form.
 */
function devshop_pull_settings_form() {
  $form['devshop_pull_ip_acl'] = array(
    '#type' => 'textarea',
    '#title' => t('Control Access by IP'),
    '#default_value' => variable_get('devshop_pull_ip_acl', 
    		     "50.57.128.197\n50.74.68.114\n108.171.174.178\n207.97.227.253\n"),
    '#rows' => 6,
    '#description' => t('Enter the IP addresses that are allowed to trigger a "Pull Code" task.  GitHub post-receive callback servers are: 50.57.128.197, 50.74.68.114, 108.171.174.178, and 207.97.227.253.  Your IP address is !ip', array('!ip' => $_SERVER['REMOTE_ADDR']))
  );
  return system_settings_form($form);
}
