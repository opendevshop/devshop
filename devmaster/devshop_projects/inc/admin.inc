<?php

/**
 * Projects Settings Page
 *
 * All code for admin interface.
 */
function devshop_projects_settings_form() {
  $form = array();

  $form['devshop_project_default_path'] = array(
    '#title' => t('Projects Base Path'),
    '#type' => 'textfield',
    '#description' => t('The folder that all projects will be created in.  Each project gets their own folder, inside that is a folder for each environment.'),
    '#default_value' => variable_get('devshop_project_default_path', '/var/aegir/projects'),
  );
  $form['devshop_project_default_drupal_path'] = array(
    '#title' => t('Default path to Drupal'),
    '#type' => 'textfield',
    '#description' => t("If index.php isn't in the root of the git repo, you can edit the 'Path to Drupal' setting on each project.  Set a default 'Path to Drupal' here. (For example, an Acquia hosted repo uses 'docroot'.)"),
    '#default_value' => variable_get('devshop_project_default_drupal_path', ''),
  );
  // @TODO: Add @environment as a replacement pattern.
  $form['devshop_project_default_base_url_pattern'] = array(
    '#title' => t('Domain Name Pattern'),
    '#type' => 'textfield',
    '#description' => t('Each environment (aegir site) gets an initial URL. You can use @project for project name and @hostname for %host. Environments are put underneath this URL as subdomains.', array('%host' => $_SERVER['hostname'])),
    '#default_value' => variable_get('devshop_project_default_base_url_pattern', '@project.@hostname'),
  );
  $form['devshop_projects_skip_settings'] = array(
    '#title' => t('Block users from editing the "Code Path", and "Path to Drupal", and "Base URL" settings.'),
    '#description' => t('When starting a new project, you can prevent users from altering these settings, forcing projects to use the defaults set on this page.  Check this box to block these settings from being changed.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_skip_settings', TRUE),
  );

  return system_settings_form($form);
}
