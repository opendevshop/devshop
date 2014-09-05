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
    $row[] = $env_name . drupal_render($form[$env_name]['git_ref']);
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
    $header['name'] = 'Name';
    $header['git_ref'] = t('Branch/Tag');
    $row[] = drupal_render($form[$env_name]['name']);
    $row[] = drupal_render($form[$env_name]['git_ref']);

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

/**
 * Preprocess page
 */
function devshop_projects_preprocess_page(&$vars){

  // On project node edit page
  if (isset($vars['node']) && $vars['node']->type == 'project' && arg(2) == 'edit'){

    $links = array();
    $node = node_load($vars['node']->nid);
    foreach ($node->project->environments as $environment){
      $links[] = l($environment->name, 'node/' . $node->nid . '/edit/' . $environment->name);
    }
    $vars['tabs2'] = theme('item_list', $links, '', 'ul', array('class' => 'tabs secondary'));
  }
}
