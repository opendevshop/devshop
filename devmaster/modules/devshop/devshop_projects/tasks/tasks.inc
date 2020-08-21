<?php
/**
 * @file devshop_project.task.inc
 * DevShop Tasks hooks.
 */

/**
 * Implements hook_hosting_tasks().
 */
function devshop_projects_hosting_tasks() {
  $tasks = array();
  $tasks['project']['verify'] = array(
    'title' => t('Verify Project'),
    'description' => t('Verifies access to the git repository, downloads branch and tag information.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'provision_save' => TRUE,
  );
  $tasks['project']['delete'] = array(
    'title' => t('Delete Project'),
    'description' => t('Delete a project and all associated sites and platforms.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
    'hidden' => TRUE,
  );
  return $tasks;
}

/**
 * Helper to provide a select list of environments for this project
 */
function devshop_projects_tasks_add_environment_to_form(&$form, $node, $description, $key = 'environment', $title = 'Environment', $type = 'radios') {
  $keys = array_keys($node->project->environments);
  $options = array_combine($keys, $keys);
  $form[$key] = array(
    '#type' => $type,
    '#title' => t('@title', array('@title' => $title)),
    '#options' => $options,
    '#default_value' => $type == 'radios' ? key($options) : array(key($options)),
    '#required' => $type == 'checkboxes',
    '#description' => t('@description', array('@description' => $description)),
  );
}
