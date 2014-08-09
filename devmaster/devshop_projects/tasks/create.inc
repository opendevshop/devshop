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

  // Look for Site to Fork or Clone

  if (arg(3) == 'fork' || arg(3) == 'clone'){

    if (arg(3) == 'fork'){
      // @TODO: Do this with hook_menu?
      $site_to_fork = arg(4);
      if ($site_to_fork && is_numeric($site_to_fork) && $site = node_load($site_to_fork)) {
        if (!empty($project->environments[$site->environment->name])){

          $form['branch_source'] = array(
            '#type' => 'item',
            '#title' => t('Fork from'),
            '#value' => t('!env environment on branch !branch', array('!env' => "<span class='environment'>{$site->environment->name}</span>", '!branch' => "<span class='branch'>{$site->environment->git_ref}</span>")),
          );
          $form['environment_source'] = array(
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
    }
    elseif (arg(3) == 'clone') {
      // @TODO: Do this with hook_menu?
      $site_to_clone = arg(4);
      if ($site_to_clone && is_numeric($site_to_clone) && $site = node_load($site_to_clone)) {

        if (!empty($project->environments[$site->environment->name])){
          $form['environment_source_display'] = array(
            '#type' => 'item',
            '#title' => t('Clone'),
            '#value' => t('!env environment', array('!env' => "<span class='environment'>{$site->environment->name}</span>")),
          );
          $form['environment_source'] = array(
            '#type' => 'hidden',
            '#value' => $site->environment->name,
          );
          $form['branch'] = array(
            '#type' => 'hidden',
            '#value' => $site->environment->git_ref,
          );
        }
      }
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
  if (in_array($params['new_branch'], $project->settings->git['branches'])){
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
  $branch = !empty($form_state['values']['parameters']['new_branch'])? $form_state['values']['parameters']['new_branch']: $form_state['values']['parameters']['branch'];
  $environment_source = $form_state['values']['parameters']['environment_source'];

  hosting_create_environment($project, $environment_name, $branch, $environment_source);

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
  global $user;
  $project = $project_node->project;

  // First, create a platform node. Only aegir properties are needed.
  $platform = new stdClass();
  $platform->type = 'platform';
  $platform->title = $project_node->title . '_' . $environment_name;
  $platform->status = 1;
  $platform->uid = $user->uid;
  $platform->name = $user->name;

  // Set servers
  // @TODO: Respect environment settings.
  $servers = hosting_get_servers('http');
  $platform->web_server = variable_get('devshop_projects_default_web_server', key($servers));;

  // If drupal path, append to code path/environment name
  if ($project->drupal_path) {
    $platform->publish_path = $project->code_path . '/' . $environment_name . '/' . $project->drupal_path;
  }
  else {
    $platform->publish_path = $project->code_path . '/' . $environment_name;
  }

  // Save the platform node.
  if ($platform = node_submit($platform)) {
    node_save($platform);
  }

  // If cloning or forking, check if source environment exists...
  if (isset($project_node->project->environments[$fork_source])) {
    $source_environment = $project_node->project->environments[$fork_source];

    $environment = new stdClass();
    $environment->git_ref = $branch;
    $environment->platform = $platform->nid;

    // Copy settings from source environment.
    $environment->settings = $source_environment->settings;

    // Save Clone/fork specific data to the environment
    $environment->settings->site_to_clone = $source_environment->site;
    $environment->settings->branch_to_fork = $source_environment->git_ref;

  }
  else {
    // Next, add the environment record.
    $environment = new stdClass();
    $environment->git_ref = $branch;
    $environment->platform = $platform->nid;
    // @TODO: Implement default environment settings.

  }

  $project->environments[$environment_name] = $environment;

  // Save the project node.
  $project_node->project = $project;
  node_save($project_node);
}
