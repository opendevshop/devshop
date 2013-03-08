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
    '#default_value' => variable_get('devshop_pull_ip_acl', DEVSHOP_PULL_DEFAULT_ALLOWED_IPS),
    '#rows' => 6,
    '#description' => t('Enter the IP addresses that are allowed to trigger a "Pull Code" task.  GitHub post-receive callback servers are: !github_ips.  Your IP address is !ip', array('!github_ips' =>variable_get('devshop_pull_ip_acl', DEVSHOP_PULL_DEFAULT_ALLOWED_IPS), '!ip' => $_SERVER['REMOTE_ADDR'])),
  );
  return system_settings_form($form);
}
