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
    'provision_save' => TRUE,
  );
  $tasks['project']['devshop-create'] = array(
    'title' => t('Create New Environment'),
    'description' => t('Creates a new environment within this project.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
    'hidden' => TRUE,
  );
  $tasks['project']['devshop-commit'] = array(
    'title' => t('Commit Features'),
    'description' => t('Recreates all Features and commits the result.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  $tasks['project']['devshop-pull'] = array(
    'title' => t('Pull Code'),
    'description' => t('Pull & verify platform code and (optionally) run update.php, clear cache, and revert features.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  $tasks['project']['delete'] = array(
    'title' => t('Delete Project'),
    'description' => t('Delete a project and all associated sites and platforms.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
    'hidden' => TRUE,
  );
  // @TODO: This task was making menu cache clears FREAK OUT.
  // We must figure out how to create a new "deploy" task.
  // This task might as well be in hosting_git.module
//  $tasks['site']['devshop-deploy'] = array(
//    'title' => t('Deploy'),
//    'access callback' => 'devshop_hosting_task_menu_access',
//    'dialog' => TRUE,
//  );
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
