<?php
/**
 * @file create.inc
 * Functions related to the "Create Environment" task.
 */

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "devshop-create" task for creating a platform and site.
 */
function hosting_task_devshop_create_form($node) {
  global $user;
  $project = $node->project;
  $branch_options = devshop_projects_git_ref_options($project);

  // Look for Site to Fork
  // @TODO: Do this with hook_menu?
  $site_to_fork = arg(3);
  if ($site_to_fork && is_numeric($site_to_fork) && $site = node_load($site_to_fork)) {
    if (!empty($project->environments[$site->environment->environment])){

      $form['branch_source'] = array(
        '#type' => 'item',
        '#title' => t('Fork from'),
        '#value' => t('!env environment on branch !branch', array('!env' => "<span class='environment'>{$site->environment->name}</span>", '!branch' => "<span class='branch'>{$site->environment->git_ref}</span>")),
      );
      $form['fork_source'] = array(
        '#type' => 'hidden',
        '#value' => $site->environment->name,
      );
      $form['new_branch'] = array(
        '#title' => t('New branch name'),
        '#description' => t('Pick a name for your new git branch.'),
        '#type' => 'textfield',
        '#size' => 60,
        '#maxlength' => 128,
        '#required' => TRUE,
      );
    }
  }

  else {
    $form['branch'] = array(
      '#title' => t('Branch/Tag'),
      '#description' => t('Choose the git branch or tag you wish to use for this new environment.  <em>Note: If you do not see all remote branches, You can !link</em>', array('!link' => l(t('Refresh branches'), 'node/' . $node->nid . '/project_verify', array('attributes' => array('class' => 'refresh-link modalframe-exclude', 'target' => '_parent'), 'query' => array('token' => drupal_get_token($user->uid)))))),
      '#type' => 'select',
      '#options' => $branch_options,
      '#required' => TRUE,
    );
  }
  $form['environment_name'] = array(
    '#title' => t('New Environment Name'),
    '#type' => 'textfield',
    '#description' => t('Enter a system name for your environment.  For consistency, you might want to match the branch name.'),
    '#required' => TRUE,
    '#field_prefix' => 'http://',
    '#field_suffix' => "." . $node->project->base_url,
    '#size' => 6,
    '#maxlength' => 64,
  );

  // @TODO: I don't think this is needed. hosting_task_devshop_create_form_submit() looks for nid, these values end up in $form_state['values']['paramters'];
  $form['project_nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  return $form;
}

/**
 * Validation hook for hosting_task_devshop_create_form()
 */
function hosting_task_devshop_create_form_validate($form, &$form_state){
  $params = $form_state['values']['parameters'];

  // Load Project
  $node = node_load($form_state['values']['nid']);
  $project = $node->project;

  // Check existence of the environment name
  if (!empty($project->environments[$params['environment_name']])){
    form_set_error('environment_name', t('%name is already in use.  Your environment name must be unique within the project.', array('%name' => $params['environment_name'])));
  }

  // Check existence of the git branch
  if (in_array($params['new_branch'], $project->data->git['branches'])){
    form_set_error('new_branch', t("The project already has a branch named %name. Choose a git branch name that doesn't already exist.", array('%name' => $params['new_branch'])));
  }

  // Check for illegal chars
  if (!preg_match('!^[a-z0-9_]+$!', $params['environment_name'])) {
    form_set_error('type', t('The environment name must contain only lowercase letters, numbers, and underscores.'));
  }

  if (isset($params['new_branch']) && !preg_match('!^[a-z0-9_-]+$!', $params['new_branch'])) {
    form_set_error('new_branch', t('The new branch name must contain only lowercase letters, numbers, dashes and underscores.'));
  }
}

/**
 * Extra submit function for hosting_task_confirm_form()
 *
 * @see devshop_projects_form_alter().  We had to add the submit hadler there.
 */
function hosting_task_devshop_create_form_submit($form, &$form_state) {

  $project = node_load($form_state['values']['nid']);
  $environment_name = $form_state['values']['parameters']['environment_name'];
  $branch = $form_state['values']['parameters']['new_branch']? $form_state['values']['parameters']['new_branch']: $form_state['values']['parameters']['branch'];
  $fork_source = $form_state['values']['parameters']['fork_source'];

  hosting_create_environment($project, $environment_name, $branch, $fork_source);

  // We are replacing hosting_confirm_form_submit here, so just do what it does,
  // minus the hosting task creation!
  $values = $form_state['values'];
  $form_state['redirect'] = 'node/' . $values['nid'];
  modalframe_close_dialog();

  // Friendly message
  drupal_set_message(t('Your environment is being created.'));
}


/**
 * API-level function for creating a new environment.
 *
 * @TODO: Add more options like web server, etc.
 * @param $project
 *   A full project node (for now. @TODO Allow project name (and/or nid) as parameters.
 * @param $environment_name
 *   A new name for the environment
 * @param $branch
 *   What git branch to track.
 * @param $fork_source
 *   If desired, the environment to fork off of. (Copy the database and create a new branch from)
 */
function hosting_create_environment($project_node, $environment_name, $branch, $fork_source = NULL) {

  $servers = hosting_get_servers('http');
  $server = variable_get('devshop_projects_default_web_server', key($servers));

  // hosting_platform fields
  $platform = new stdClass;
  $platform->title = $project_node->title . '_' . $environment_name;
  $platform->publish_path = $project->code_path . '/' . $environment_name;
  $platform->web_server = $server;
  $platform->git_branch = $branch;
  $platform->project = $project->title;
  $platform->environment = $environment_name;
  $platform->drupal_path = $project->drupal_path;

  if ($project->drupal_path) {
    $platform->publish_path = $project->code_path . '/' . $environment_name . '/' . $project->drupal_path;
  }

  //Check if we are forking another site.
  if (isset($project_node->project->environments[$fork_source])) {
    $platform->clone_nid = $project_node->project->environments[$fork_source]['site'];
    $platform->git_branch = $branch;
    $platform->old_branch = $project_node->project->environments[$fork_source]['git_branch'];
  }

  watchdog('debug', 'Attempting to create: ' . print_r($platform,1));

  // Create the platform node. The platform and site creation handle it from there.
  $platform_node = _devshop_projects_node_create('platform', $platform);

  return $platform_node;
}
