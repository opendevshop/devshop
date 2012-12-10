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
  /*
  @TODO: Refactor custom forms to work with hosting_confirm_form()
  $tasks['project']['delete'] = array(
    'title' => t('Delete Project'),
    'description' => t('Delete a project and all associated sites and platforms.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  */
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

/**
 * Extra submit function for hosting_task_confirm_form()
 *
 * @see devshop_projects_form_alter().  We had to add the submit hadler there.
 */
function hosting_task_devshop_create_form_submit($form, &$form_state) {

  $project = node_load($form_state['values']['nid']);
  $platform_name = $form_state['values']['parameters']['platform_name'];
  $branch = $form_state['values']['parameters']['branch'];
  $servers = hosting_get_servers('http');
  $server = variable_get('devshop_projects_default_web_server', key($servers));

  // hosting_platform fields
  $platform = new stdClass;
  $platform->title = $project->title . '_' . $platform_name;
  $platform->publish_path = $project->code_path . '/' . $platform_name;
  $platform->web_server = $server;
  $platform->git_branch = $branch;
  $platform->project = $project->title;
  $platform->environment = $platform_name;

  watchdog('debug', 'Form state: ' . print_r($form_state['values'],1));

  watchdog('debug', 'Attempting to create: ' . print_r($platform,1));

  $platform_node = _devshop_projects_node_create('platform', $platform);


}
