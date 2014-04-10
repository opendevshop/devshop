<?php

/**
 * Implements hook_hosting_theme()
 */
function devshop_projects_theme(){
  return array(
    'devshop_projects_settings_form' => array(
      'arguments' => array(
        'form' => NULL,
      ),
    ),
    'devshop_projects_create_settings_form' => array(
      'arguments' => array(
        'form' => NULL,
      ),
    ),
  );
}

/**
 * Theme function for environments settings
 */
function theme_devshop_projects_settings_form($form) {
  $rows = array();
  $header = array();
  $header[] = t('Environment');
  foreach (element_children($form) as $env_name) {
    $row = array();
    $row[] = $env_name;
    $row[] = drupal_render($form[$env_name]['git_ref']);
    $header['git_ref'] = t('Branch/Tag');
    foreach(element_children($form[$env_name]['settings']) as $setting){
      if (!isset($header[$setting])){
        $header[$setting] = $form[$env_name]['settings'][$setting]['#title'];
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name]['settings'][$setting]);
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('id' => 'project-settings-table'));
  return $output; 
}

/**
 * Theme function for create environments settings
 * @TODO: Fold into theme_devshop_projects_settings_form()
 */
function theme_devshop_projects_create_settings_form($form) {
  $rows = array();
  $header = array();
  foreach (element_children($form) as $env_name) {
    $row = array();
    $header['title'] = 'Name';
    $row[] = drupal_render($form[$env_name]['title']);

    foreach(element_children($form[$env_name]['settings']) as $setting){
      if (!isset($header[$setting])){
        $header[$setting] = $form[$env_name]['settings'][$setting]['#title'];
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name]['settings'][$setting]);
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('id' => 'project-settings-table'));
  $output .= '<p>'. t('Create as many new environments as you would like. For example: "dev", "test", and "live". You can create more later on if needed.') .'</p>';

  return $output; 
}
