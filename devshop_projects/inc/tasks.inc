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
    'hidden' => TRUE,
    'nid' => NULL,  // This silly hack hides verify... hidden won't work!
  );
  $tasks['project']['devshop-create'] = array(
    'title' => t('Create New Environment'),
    'description' => t('Creates a new site & platform within this project.'),
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
  $tasks['project']['devshop-sync'] = array(
    'title' => t('Sync Data'),
    'description' => t('Sync database and files from another site and (optionally) run update.php, clear cache, and revert features.'),
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

  return $tasks;
}

/**
 * Helper to provide a select list of environments for this project
 */
function devshop_projects_tasks_add_environment_to_form(&$form, $node, $description, $key = 'environment', $title = 'Environment', $type = 'radios') {
  // @TODO: Add a check here.  Sometimes we want to limit this list.

  $options = array_combine($node->project_objects['site'], $node->project_objects['site']);
  $form[$key] = array(
    '#type' => $type,
    '#title' => t($title),
    '#options' => $options,
    '#default_value' => $type == 'radios'? key($options): array(key($options)),
    '#required' => $type == 'checkboxes',
    '#description' => $description,
  );
}

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "devshop-create" task for creating a platform and site.
 */
function hosting_task_devshop_create_form($node) {
  $branch_options = array_combine($node->git_branches, $node->git_branches);

  $site_nid = arg(3);
  $site = NULL;

  if ($site_nid && is_numeric($site_nid)) {
    //Sanitize data.
    $site_nid = check_plain($site_nid);

    //Check is a valid project site.
    if (!empty($node->project_objects['site'])) {
      foreach($node->project_objects['site'] as $nid => $alias) {
        if ($nid == $site_nid) {
          $site = node_load($nid);
        }
      }
    }
  } 

  if ($site) {
    $form['branch'] = array(
      '#type' => 'hidden',
      '#value' => $site->git_branch,
    );
    $form['clone_nid'] = array(
      '#type' => 'hidden',
      '#value' => $site->nid,
    );
    $form['new_branch'] = array(
      '#title' => t('New branch name'),
      '#description' => t('Choose the new Git branch you with to use for this new platform.'),
      '#type' => 'textfield',
      '#size' => 60,
      '#maxlength' => 128,
      '#required' => TRUE,
    );
  }
  else {
    $form['branch'] = array(
      '#title' => t('Branch'),
      '#description' => t('Choose the Git branch you with to use for this new platform.  <em>Note: If you do not see all remote branches, re-verify your project.</em>'),
      '#type' => 'select',
      '#options' => $branch_options,
      '#required' => TRUE,
    );
  }
  // @TODO: Add "create branch" functionality.
  $form['platform_name'] = array(
    '#title' => t('Environment Name'),
    '#type' => 'textfield',
    '#description' => t('Enter a system name for your environment.  For consistency, you should make this match the branch name.'),
    '#required' => TRUE,
  );
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

  // Check for existing environments for this project.
  $object_nid = db_result(db_query('SELECT object_nid FROM {hosting_devshop_project_object} WHERE project_nid = %d AND object_type = "%s" AND environment = "%s"', $params['project_nid'], 'platform', $params['platform_name']));
  if ($object_nid) {
    form_set_error('platform_name', t('%name is already in use.  Your environment name must be unique within the project.', array('%name' => $params['platform_name'])));
  }

  // Check for illegal chars
  if (!preg_match('!^[a-z0-9_]+$!', $params['platform_name'])) {
    form_set_error('type', t('The environment name must contain only lowercase letters, numbers, and underscores.'));
  }

  if (isset($params['new_branch']) && !preg_match('!^[a-z0-9_]+$!', $params['platform_name'])) {
    form_set_error('new_branch', t('The new branch name must contain only lowercase letters, numbers, and underscores.'));
  }
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
  $platform->drupal_path = $project->drupal_path;

  if ($project->drupal_path) {
    $platform->publish_path = $project->code_path . '/' . $platform_name . '/' . $project->drupal_path;
  }

  //Check if is need to clone and create a new branch
  if (isset($form_state['values']['parameters']['clone_nid'])) {
    $platform->clone_nid = $form_state['values']['parameters']['clone_nid'];
    $platform->git_branch = $form_state['values']['parameters']['new_branch'];
    $platform->old_branch = $branch;
  }

  watchdog('debug', 'Attempting to create: ' . print_r($platform,1));

  // Create the platform node
  $platform_node = _devshop_projects_node_create('platform', $platform);

  // We are replacing hosting_confirm_form_submit here, so just do what it does,
  // minus the hosting task creation!
  $values = $form_state['values'];
  $form_state['redirect'] = 'node/' . $values['nid'];
  modalframe_close_dialog();
  
  // Friendly message
  drupal_set_message(t('Your environment is being created.'));
}


/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Commit" task.
 */
function hosting_task_devshop_commit_form($node) {

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the environment to commit features from.'));

  $descr = 'A message describing this commit. Too see a diff output off all of the features, ';

  if (_devshop_projects_site_has_module($node, 'features_diff')) {
    $descr .= 'click ' . l(t('here.'), "http://$node->hosting_name/features/diff/all", array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))) . ' Be patient. It takes a few moments for the diffs to be generatred.';
  }
  else {
    $descr .= 'enable the Features Diff module for this site, Verify the site, and select this task again.';
  }

  $form['message'] = array(
    '#title' => t('Commit Message'),
    '#type' => 'textarea',
    '#description' => $descr,
  );
  $form['push'] = array(
    '#title' => t('Push code after commit?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  $form['revert'] = array(
    '#title' => t('Force revert features after commit?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  // @TODO: Provide a DIFF display to give the user an idea of what has changed.
  return $form;
}

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Pull Code" task.
 */
function hosting_task_devshop_pull_form($node) {

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the environments to pull code to.'), 'environments', 'Environments', 'checkboxes');

  /*
   *$form['environments'] = array(
    '#title' => t('list of environments'),
    '#type' => 'textfield',
    '#default_value' => 'dev test',
  );

  */

  $form['update'] = array(
    '#title' => t('Run update.php after code pull?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );

  if (_devshop_projects_project_has_module($node, 'features')){
    $form['revert'] = array(
      '#title' => t('Revert all features after code pull?'),
      '#type' => 'checkbox',
      '#default_value' => 1,
    );
  }
  $form['cache'] = array(
    '#title' => t('Clear cache after code pull?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  $form['force'] = array(
    '#title' => t('Force'),
    '#description' => t('Force to execute update, cache if are enabled'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );

  // Add validator for environments
  //$form['#validate'] = array('hosting_task_devshop_pull_form');
  return $form;
}

/**
 * Extra submit function for hosting_task_confirm_form()
 *
 * @see devshop_projects_form_alter().  We had to add the submit hadler there.
 */
function hosting_task_devshop_pull_form_validate($form, &$form_state) {
  $value = implode(' ', array_filter($form_state['values']['parameters']['environments']));
  form_set_value($form['parameters']['environments'], $value, $form_state);
}


/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Sync Content" task.
 */
function hosting_task_devshop_sync_form($node) {
  // Get nid
  $nid = $node->nid;

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the source environment.'), 'source', 'Source');
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the destination environment.  <em>Data from this environment will be DESTROYED.</em>'), 'destination', 'Destination');

  $form['source']['#prefix'] = '<div class="sync-envs"><div class="source">';
  $form['source']['#suffix'] = '</div>';
  $form['destination']['#prefix'] = '<div class="destination">';
  $form['destination']['#suffix'] = '</div></div>';

  $form['database'] = array(
    '#title' => t('Copy database from source to destination.'),
    '#type' => 'checkbox',
    '#default_value' => 1,
    '#prefix' => '<fieldset><legend>What to sync</legend>',
  );

  $form['files'] = array(
    '#title' => t('Copy files folder from source to destination.'),
    '#type' => 'checkbox',
    '#default_value' => 0,
    '#suffix' => '</fieldset>',
  );

  
  $form['pull'] = array(
    '#title' => t('Pull code on Destination'),
    '#type' => 'checkbox',
    '#default_value' => 1,
    '#prefix' => '<fieldset><legend>Actions</legend>',
  );
  $form['update'] = array(
    '#title' => t('Run update.php on Destination'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  if (_devshop_projects_project_has_module($node, 'features')){
    $form['revert'] = array(
      '#title' => t('Revert all features on Destination'),
      '#type' => 'checkbox',
      '#default_value' => $has_features,
      '#access' => $has_features,
    );
  } else{
    // I think its better UI just to hide it? If they need features it will be enabled!
    //$form['actions']['revert'] = array(
    //  '#title' => t('<em>Revert all features on Destination after content sync?</em>'),
    //  '#type' => 'checkbox',
    //  '#description' => t('This site doesn\'t have features.module enabled. Please enable and then "Verify" the site.'),
    //  '#default_value' => FALSE,
    //  '#disabled' => TRUE,
    //  );
  }
  $form['cache'] = array(
    '#title' => t('Clear cache on Destination'),
    '#type' => 'checkbox',
    '#default_value' => 1,
    '#suffix' => '</fieldset>',
  );
  $form['#validate'][] = 'hosting_task_devshop_sync_form_validate';
  return $form;
}

/**
 * Validation for hosting_task_devshop_sync_form()
 */
function hosting_task_devshop_sync_form_validate(&$form, &$form_state){
  // Can't sync to self
  if ($form_state['values']['parameters']['source'] == $form_state['values']['parameters']['destination']){
    form_set_error('source', t('The source cannot be the same as the destination.'));
  }
}
