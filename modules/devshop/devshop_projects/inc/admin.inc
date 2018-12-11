<?php

/**
 * Projects Settings Page
 *
 * All code for admin interface.
 */
function devshop_projects_settings_form($form, &$form_state) {
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

  $form['urls']['devshop_projects_allow_changing_project_git_url'] = array(
    '#title' => t("Allow changing a project's Default Git URL."),
    '#description' => t("When editing a project, do not allow changes to the Git URL."),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_changing_project_git_url', TRUE),
  );
  $form['urls']['devshop_projects_allow_custom_base_url'] = array(
    '#title' => t('Allow custom Environment Domain Name Pattern per project.'),
    '#description' => t('If enabled each project can set an Environment URL Pattern.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_custom_base_url', FALSE),
  );
  $form['urls']['devshop_project_environment_url_pattern'] = array(
    '#title' => t('Environment Domain Name Pattern'),
    '#type' => 'textfield',
    '#description' => t("Each environment will have a system domain name generated for it based on it's name. Use @project for project name, @hostname for '%host', @environment for the environment's name.", array('%host' => $_SERVER['SERVER_NAME'])),
    '#default_value' => variable_get('devshop_project_environment_url_pattern', '@project.@environment.@hostname'),
    '#element_validate' => array(
      'devshop_project_settings_validate_environment_url_pattern',
    ),
  );

//  $form['support'] = array(
//      '#type' => 'fieldset',
//      '#title' => t('DevShop Support'),
//  );
//  $form['support']['devshop_support_widget_enable'] = array(
//    '#title' => t('Show Help Widget'),
//    '#description' => t('Uncheck this box if you want to hide the Help widget that appears at the bottom right of the page.'),
//    '#type' => 'checkbox',
//    '#default_value' => variable_get('devshop_support_widget_enable', TRUE),
//  );

  // Server settings.
  $form['server'] = array(
      '#type' => 'fieldset',
      '#title' => t('DevShop Server'),
  );
  $form['server']['devshop_public_key'] = array(
    '#title' => t('Aegir User Public Key'),
    '#description' => t('The public key of the aegir user on this server. If you change the SSH keys located at <code>/var/aegir/.ssh/id_rsa.pub</code>, then you should update this variable. This variable is for user reference only. If using GitHub API, then devshop will check to make sure the server has access using this public key.'),
    '#type' => 'textarea',
    '#default_value' => variable_get('devshop_public_key', ''),
  );
  return system_settings_form($form);
}

/**
 * Validate that the default environment domain pattern has @environment and @project.
 */
function devshop_project_settings_validate_environment_url_pattern($element, &$form_state, $form) {
  if (strpos($form_state['values']['devshop_project_environment_url_pattern'], '@environment') === FALSE || strpos($form_state['values']['devshop_project_environment_url_pattern'], '@project') === FALSE) {
    form_error($element, t('The placeholders @project and @environment must be in the Environment Domain Name Pattern.'));
  }
}