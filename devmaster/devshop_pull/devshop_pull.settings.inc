<?php

/**
 * Page callback for the settings page.
 */
function devshop_pull_settings_page(){
  $output .= drupal_get_form('devshop_pull_settings_form');
  $output .= drupal_get_form('devshop_pull_reset_form');
  return $output;
}

/**
 * General settings form.
 */
function devshop_pull_settings_form() {
  $form['devshop_pull_ip_acl'] = array(
    '#type' => 'textarea',
    '#title' => t('Control Access by IP'),
    '#default_value' => variable_get('devshop_github_ip_acl', 
    		     "50.57.128.197\n50.74.68.114\n108.171.174.178\n207.97.227.253\n"),
    '#rows' => 6,
    '#description' => t('Enter the IP addresses that are allowed to trigger a "Pull Code" task.  GitHub post-receive callback servers are: 50.57.128.197, 50.74.68.114, 108.171.174.178, and 207.97.227.253.  Your IP address is !ip', array('!ip' => $_SERVER['REMOTE_ADDR']))
  );
  return system_settings_form($form);
}

/**
 * Tool to enable pull on all projects.
 */
function devshop_pull_reset_form() {
  $form['note'] = array(
    '#value' => '<h2>' . t('Reset all Projects') . '</h2>' . '<p>' . t('If you have a lot of projects and need to turn them all on, you can do so here.') . '</p>',
  );
  $form['pull_method'] = array(
    '#title' => 'Automatic Git Pull Method',
    '#type' => 'radios',
    '#default_value' => DEVSHOP_PULL_DISABLED,
    '#options' => array(
DEVSHOP_PULL_DISABLED => t('Pull disabled.'),
DEVSHOP_PULL_QUEUE => t('Pull on queue (every minute).'),
DEVSHOP_PULL_CALLBACK => t('Pull on URL Callback (ie. GitHub Webhook)'),

    ),
  );
  
  $form['enable_dev_sites'] = array(
    '#title' => t('On all dev sites, Enable Pull on Commit and Reset Hard.'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Enable Pull for all projects'),
  );
  return $form;
}


/**
 * Submit function for devshop_pull_reset_form()
 */
function devshop_pull_reset_form_submit($form, &$form_state)  {
  /*
  // Clear out tables
  db_query('TRUNCATE {hosting_devshop_pull_platforms}');
  db_query('TRUNCATE {hosting_devshop_pull_projects}');
  
  // Find all projects
  $query = db_query('SELECT nid FROM node WHERE type="project" AND status=1;');
  while($node = db_fetch_object($query)){
    $pull_data = array();
    $pull_data['project_nid'] = $node->nid;
    $pull_data['pull_method'] = $form_state['values']['pull_method'];
    drupal_write_record('hosting_devshop_pull_projects', $pull_data);
  }
  
  // Loop through and insert data into tables
  
  // Find all dev sites
  
  // Loop through and insert data into tables
  
  drupal_set_message(t('Set all projects to !method', array('!method' => $form_state['values']['pull_method'])));
  */
  drupal_set_message('Not yet implemented!');
}