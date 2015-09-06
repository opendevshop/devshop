<?php

/**
 * Projects Settings Page
 *
 * All code for admin interface.
 */
function devshop_projects_settings_form() {
  $form = array();

  $form['paths'] = array(
    '#type' => 'fieldset',
    '#title' => t('Paths'),
  );
  $form['paths']['devshop_projects_allow_custom_code_path'] = array(
    '#title' => t('Allow custom code path per project.'),
    '#description' => t('Allow each project to have a custom "Code Path". If not checked, project paths are set as "/var/aegir/projects/{PROJECT_NAME}.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_custom_code_path', FALSE),
  );
  $form['paths']['devshop_project_base_path'] = array(
    '#title' => t('Projects Base Path'),
    '#type' => 'textfield',
    '#description' => t('The default base path that all projects will be created in.  Projects each get their own folder inside this path.'),
    '#default_value' => variable_get('devshop_project_base_path', '/var/aegir/projects'),
  );
  $form['paths']['devshop_project_default_drupal_path'] = array(
    '#title' => t('Default path to Drupal'),
    '#type' => 'textfield',
    '#description' => t("If index.php isn't in the root of the git repo, you can edit the 'Path to Drupal' setting on each project.  Set a default 'Path to Drupal' here. (For example, an Acquia hosted repo uses 'docroot'.)"),
    '#default_value' => variable_get('devshop_project_default_drupal_path', ''),
  );


  $form['urls'] = array(
    '#type' => 'fieldset',
    '#title' => t('URLs'),
  );

  $form['urls']['devshop_projects_allow_custom_base_url'] = array(
    '#title' => t('Allow custom base URL per project.'),
    '#description' => t('All environments are made available at http://{ENVIRONMENT}.BASE_URL. Check this box to make the Base URL configurable per project.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_custom_base_url', FALSE),
  );
  // @TODO: Add @environment as a replacement pattern.
  $form['urls']['devshop_project_environment_url_pattern'] = array(
    '#title' => t('Domain Name Pattern'),
    '#type' => 'textfield',
    '#description' => t('Each environment (aegir site) gets an initial URL. You can use @project for project name and @hostname for "%host". Environments are put underneath this URL as subdomains.', array('%host' => $_SERVER['SERVER_NAME'])),
    '#default_value' => variable_get('devshop_project_environment_url_pattern', '@environment.@project.@hostname'),
  );

  $form['support'] = array(
      '#type' => 'fieldset',
      '#title' => t('DevShop Support'),
  );
  $form['support']['devshop_support_widget_enable'] = array(
    '#title' => t('Show Help Widget'),
    '#description' => t('Uncheck this box if you want to hide the Help widget that appears at the bottom right of the page.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_support_widget_enable', TRUE),
  );
  return system_settings_form($form);
}
