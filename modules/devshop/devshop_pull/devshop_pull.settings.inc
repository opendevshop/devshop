<?php

/**
 * Page callback for the settings page.
 */
function devshop_pull_settings_page() {
  return drupal_get_form('devshop_pull_settings_form');
}

/**
 * General settings form.
 */
function devshop_pull_settings_form($form) {
  $form['devshop_pull_ip_acl'] = array(
    '#type' => 'textarea',
    '#title' => t('Control Access by IP'),
    '#default_value' => variable_get('devshop_pull_ip_acl', DEVSHOP_PULL_DEFAULT_ALLOWED_IPS),
    '#rows' => 6,
  );

  // we have a few bullet points
  $items = array();
  $items[] =  t('Enter the IP addresses that are allowed to trigger a "Deploy" tasks. You may specify address ranges using CIDR notation (e.g. 192.168.1.0/24).');
  $items[] = t('GitHub post-receive callback servers are: %github_ips.', array('%github_ips' => "204.232.175.64/27 and 192.30.252.0/22"));
  $items[] = t('BitBucket post-receive callback servers are: %bitbucket_ips.', array('%bitbucket_ips' => '104.192.143.192/28 and 104.192.143.208/28'));
  $items[] = t('Your local computer\'s IP address is %ip.  ', array('%ip' => $_SERVER['REMOTE_ADDR']));

  // If there was a last ip logged, display it here so its easy to add to the list.
  $last_ip = variable_get('devshop_pull_last_ip', '');
  if ($last_ip) {
    $items[] = t('The last IP to attempt a commit notification was %ip', array('%ip' => $last_ip));
  }
  else {
    $items[] = t('No requests ever detected.  If you add the trigger URL for a project to your git repo host, the IP will be logged and displayed here.');
  }

  if (variable_get('devshop_pull_last_status', '') == DEVSHOP_PULL_STATUS_ACCESS_DENIED) {
    $items[] = t('The last incoming webhook was denied because it is not in this list.');
  }

  $form['devshop_pull_ip_acl']['#description'] = theme('item_list', array('items' => $items));

  return system_settings_form($form);
}
