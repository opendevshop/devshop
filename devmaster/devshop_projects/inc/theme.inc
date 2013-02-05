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
  );
}
/**
 * Theme function for environments settings
 */
function theme_devshop_projects_settings_form($form){
  $rows = array();
  $header = array();
  $header[] = t('Environment');
  foreach (element_children($form) as $env_name) {
    $row = array();
    $row[] = $env_name;
    foreach(element_children($form[$env_name]) as $setting){
      if (!isset($header[$setting])){
        $header[$setting] = $form[$env_name][$setting]['#title'];
      }
      $form[$env_name][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name][$setting]); 
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('id' => 'project-settings-table'));
  return $output; 
}