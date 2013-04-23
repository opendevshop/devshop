<?php

/**
 * Projects Settings Page
 *
 * All code for admin interface.
 */
function devshop_projects_settings_form() {
  $form = array();

  $form['devshop_project_default_path'] = array(
    '#title' => t('Default Project Path'),
    '#type' => 'textfield',
    '#description' => t('Used in Code Path'),
    '#default_value' => variable_get('devshop_project_default_path', '/var/aegir/projects'),
  );
  $form['devshop_project_default_drupal_path'] = array(
    '#title' => t('Default Project Drupal Path'),
    '#type' => 'textfield',
    '#description' => t('Used in Drupal Code Path'),
    '#default_value' => variable_get('devshop_project_default_drupal_path', ''),
  );
  $form['devshop_project_default_base_url_pattern'] = array(
    '#title' => t('Default Primary Domain Pattern'),
    '#type' => 'textfield',
    '#description' => t('Used in Primary Domain. You can use @project for project name and @hostname for server name.'),
    '#default_value' => variable_get('devshop_project_default_base_url_pattern', '@project.@hostname'),
  );
  $form['devshop_projects_skip_settings'] = array(
    '#title' => t('Skip Settings Page'),
    '#description' => t('when starting a new project'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_skip_settings', TRUE),
  );

  return system_settings_form($form);
}
