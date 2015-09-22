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
          $form['action'] = array(
            '#type' => 'value',
            '#value' => 'fork'
          );
          $form['fork_display'] = array(
            '#type' => 'item',
            '#title' => t('Fork from'),
            '#value' => t('!env environment on branch !branch', array('!env' => "<span class='environment'>{$site->environment->name}</span>", '!branch' => "<span class='branch'>{$site->environment->git_ref}</span>")),
          );
          $form['environment_source'] = array(
            '#type' => 'value',
            '#value' => $site->environment->name,
          );
          $form['git_ref'] = array(
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
          $form['action'] = array(
            '#type' => 'value',
            '#value' => 'clone'
          );
          $form['environment_source_display'] = array(
            '#type' => 'item',
            '#title' => t('Clone'),
            '#weight' => -1000,
            '#value' => t('!env environment', array('!env' => "<span class='environment'>{$site->environment->name}</span>")),
          );
          $form['environment_source'] = array(
            '#type' => 'value',
            '#value' => $site->environment->name,
          );
          $form['git_ref'] = array(
            '#type' => 'value',
            '#value' => $site->environment->git_ref,
          );
        }
      }
    }
  }
  else {
    $form['action'] = array(
      '#type' => 'value',
      '#value' => 'create',
    );
    $form['git_ref'] = array(
      '#title' => t('Branch/Tag'),
      '#description' => t('Choose the git branch or tag you wish to use for this new environment.  <em>Note: If you do not see all remote branches, You can !link</em>', array('!link' => l(t('Refresh branches'), 'node/' . $node->nid . '/project_verify', array('attributes' => array('class' => 'refresh-link modalframe-exclude', 'target' => '_parent'), 'query' => array('token' => drupal_get_token($user->uid)))))),
      '#type' => 'select',
      '#options' => $branch_options,
      '#required' => TRUE,
      '#weight' => -99,
    );
    $form['environment_source'] = array(
      '#type' => 'value',
      '#value' => NULL,
    );
  }
  $form['environment_name'] = array(
    '#title' => t('New Environment Name'),
    '#type' => 'textfield',
    '#description' => t('Enter a system name for your environment.  For consistency, you might want to match the branch name.'),
    '#required' => TRUE,
    '#field_prefix' => 'http://',
    '#field_suffix' => "." . $node->project->base_url,
    '#size' => 10,
    '#maxlength' => 64,
    '#weight' => -100,
  );

  if ($project->settings->live['environment_aliases'] && !empty($node->project->settings->live['live_domain'])) {
    $form['environment_alias'] = array(
      '#title' => t('Additional Domain'),
      '#type' => 'item',
      '#description' => t('Your environment will get an additional Domain.'),
      '#value' => 'http://ENVIRONMENTNAME.' . $node->project->settings->live['live_domain'],
    );
  }

  // Server Selection
  // Choose database server.
  $servers = hosting_get_servers('db');
  if (sizeof($servers) > 1) {
    $form['db_server'] = array(
      '#type' => 'radios',
      '#title' => t('Database server'),
      '#required' => true,
      '#description' => t(
        'The database server the site will use to host its content.'
      ),
      '#options' => $servers,
      '#default_value' => $project->settings->default_environment['db_server'],
    );
  }

  $servers = hosting_get_servers('http');
  if (sizeof($servers) > 1) {
    $form['web_server'] = array(
      '#type' => 'radios',
      '#title' => t('Web server'),
      '#description' => t('The web server the sites will be hosted on.'),
      '#options' => $servers,
      '#default_value' => $project->settings->default_environment['web_server'],
    );
  }

  // Load in environment settings
  // This allows modules that add settings to environments form to do so here as well.
  $form_state = array();
  devshop_projects_form_alter($form, $form_state, 'site_node_form');
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
 * @see devshop_projects_form_alter().  We had to add the submit handler there.
 */
function hosting_task_devshop_create_form_submit($form, &$form_state) {
  $project_node = node_load($form_state['values']['nid']);
  $environment_name = $form_state['values']['parameters']['environment_name'];
  $environment_settings = $form_state['values']['parameters']['environment']['settings'];
  $environment_settings['db_server'] = $form_state['values']['parameters']['db_server'];
  $environment_settings['web_server'] = $form_state['values']['parameters']['web_server'];

  $action = $form_state['values']['parameters']['action'];
  $git_ref = $form_state['values']['parameters']['git_ref'];
  $environment_to_clone = $form_state['values']['parameters']['environment_source'];

  hosting_create_environment($project_node->project, $environment_name, $git_ref, $environment_to_clone, $environment_settings, $action);

  // We are replacing hosting_confirm_form_submit here, so just do what it does,
  // minus the hosting task creation!
  $values = $form_state['values'];
  $form_state['redirect'] = 'node/' . $values['nid'];

  // Friendly message
  drupal_get_messages('status');
  drupal_set_message(t('Your environment is being created.'));
}

/**
 * API-level function for creating a new environment.
 *
 * @TODO: Add more options like web server, etc.
 * @param $project
 *   A full project node (for now. @TODO Allow project name (and/or nid) as parameters.
 * @param $new_environment_name
 *   A new name for the environment
 * @param $new_environment_branch
 *   What git branch to track. If FALSE, will not fork.
 * @param $source_environment
 *   If desired, the environment to clone. (Copy the database and create a new branch from)
 */
function hosting_create_environment($project, $new_environment_name, $new_environment_branch = NULL, $source_environment = NULL, $settings = NULL, $action = 'create') {
  $settings = (object) $settings;
  global $user;

  // First, create a platform node. Only aegir properties are needed.
  $platform = new stdClass();
  $platform->type = 'platform';
  $platform->title = $project->name . '_' . $new_environment_name;
  $platform->status = 1;
  $platform->uid = $user->uid;
  $platform->name = $user->name;

  // Set web server to the project default.
  if ($settings->web_server) {
    $platform->web_server = $settings->web_server;
  }
  else {
    $platform->web_server = $project->settings->default_environment['web_server'];
  }

  // If drupal path, append to code path/environment name
  if ($project->drupal_path) {
    $platform->publish_path = $project->code_path . '/' . $new_environment_name . '/' . $project->drupal_path;
  }
  else {
    $platform->publish_path = $project->code_path . '/' . $new_environment_name;
  }

  // Save the platform node.
  if ($platform = node_submit($platform)) {
    node_save($platform);
  }

  // If no new branch specified and fork source is present, set branch to forked environment's branch.
  if (empty($new_environment_branch) && $source_environment) {
    $branch = $project->environments[$source_environment]->git_ref;
  }
  // If no new branch and fork source not mentioned, return false.
  elseif (empty($new_environment_branch) && empty($source_environment)) {
    return FALSE;
  }
  else {
    $branch = $new_environment_branch;
  }

  // If cloning or forking, check if source environment exists...
  if (isset($project->environments[$source_environment])) {
    $source_environment = $project->environments[$source_environment];

    $environment = new stdClass();
    $environment->name = $new_environment_name;
    $environment->git_ref = $branch;
    $environment->platform = $platform->nid;

    // Copy settings from source environment.
    $environment->settings = $source_environment->settings;

    // Save Clone/fork specific data to the environment
    $environment->settings->site_to_clone = $source_environment->site;

    if ($action == 'fork') {
      $environment->settings->branch_to_fork = $source_environment->git_ref;
    }
  }
  else {
    // Next, add the environment record.
    $environment = new stdClass();
    $environment->name = $new_environment_name;
    $environment->git_ref = $branch;
    $environment->platform = $platform->nid;

    // Use settings passed in to this function.
    $environment->settings = $settings;
  }

  $environment->last_task = array(
      'status' => HOSTING_TASK_QUEUED,
      'type' => 'verify',
      'nid' => $platform->nid,
      'executed' => 0,
  );

  db_query('INSERT INTO {hosting_devshop_project_environment} (project_nid, name, git_ref, platform, settings, last_task) VALUES (%d, "%s", "%s", %d, "%s", "%s");', $project->nid, $environment->name, $environment->git_ref, $environment->platform, serialize($environment->settings), serialize($environment->last_task));

}
