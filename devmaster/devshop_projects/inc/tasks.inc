<?php
/**
 * @file devshop_project.task.inc DevShop Project Task related hooks and
 * support function include file.
 */

/**
 * Implementation of hook_hosting_tasks()
 */
function devshop_projects_hosting_tasks() {
  $tasks = array();
  $tasks['project']['verify'] = array(
    'title' => t('Verify Project'),
    'description' => t('Verifies access to the git repository, downloads branch and tag information.'),
    'provision_save' => TRUE,
    'access callback' => 'devshop_hosting_task_menu_access',
  );
  $tasks['project']['devshop-create'] = array(
    'title' => t('Create New Platform'),
    'description' => t('Creates a new platform within this project.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  $tasks['project']['delete'] = array(
    'title' => t('Delete Project'),
    'description' => t('Delete a project and all associated sites and platforms.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  return $tasks;
}

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "devshop-create" task for creating a platform and site.
 */
function hosting_task_devshop_create_form($node) {
  $branch_options = array_combine($node->git_branches, $node->git_branches);
  $form['branch'] = array(
    '#title' => t('Branch'),
    '#description' => t('Choose the Git branch you with to use for this new platform.  <em>Note: If you do not see all remote branches, re-verify your project.</em>'),
    '#type' => 'select',
    '#options' => $branch_options,
    '#required' => TRUE,
  );
  // @TODO: Add "create branch" functionality.
  $form['platform_name'] = array(
    '#title' => t('Platform Name'),
    '#type' => 'textfield',
    '#description' => t('Enter the system name of your platform.  For consistency, you should make this match the branch name.'),
    '#required' => TRUE,
  );
  /* @TODO: Let devshop_pull handle this.
  $form['pull'] = array(
    '#title' => t('Enable Pull on Commit'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  */
  $form['#submit'][] = 'hosting_task_devshop_create_form_submit';
  return $form;
}
