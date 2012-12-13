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
    'title' => t('Create New Site'),
    'description' => t('Creates a new site & platform within this project.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
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
    'title' => t('Sync Content'),
    'description' => t('Sync content from another site and (optionally) run update.php, clear cache, and revert features.'),
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
 * Helper to provide a select list of environments for this project
 */
function devshop_projects_tasks_add_environment_to_form(&$form, $node, $description, $key = 'environment', $title = 'Environment') {
  // @TODO: Add a check here.  Sometimes we want to limit this list.
  $options = array_combine($node->project_objects['site'], $node->project_objects['site']);
  $form[$key] = array(
    '#type' => 'radios',
    '#title' => t($title),
    '#options' => $options,
    '#default_value' => key($options),
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

  // Create the platform node
  $platform_node = _devshop_projects_node_create('platform', $platform);

  // We are replacing hosting_confirm_form_submit here, so just do what it does,
  // minus the hosting task creation!
  $values = $form_state['values'];
  $form_state['redirect'] = 'node/' . $values['nid'];
  modalframe_close_dialog();
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
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the environment to pull code to.'));
  
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
  return $form;
}

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Sync Content" task.
 */
function hosting_task_devshop_sync_form($node) {
/* Disabled until we add support for custom aliases
  $form['source'] = array(
    '#title' => t('Source site alias'),
    '#type' => 'textfield',
    '#description' => t('Enter a site alias to sync from.  The alias must exist in the local system.'),
  );
  */
  // Get nid
  $nid = $node->nid;

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the source environment.'), 'source', 'Source');
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the destination environment.'), 'destination', 'Destination');
    
  $form['note'] = array(
    '#value' => '<p>'. t('This will DESTROY the database for <strong>Destination</strong> and replace it with the database for the selected <strong>Source</strong>.', array('!site' => l($node->title, "node/$nid"))) . '</p>',
    '#type' => 'markup',
    '#weight' => 100,
  );
  $form['pull'] = array(
    '#title' => t('Pull code on Destination before content sync?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  $form['update'] = array(
    '#title' => t('Run update.php on Destination after content sync?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  if (_devshop_projects_project_has_module($node, 'features')){
    $form['revert'] = array(
      '#title' => t('Revert all features on Destination after content sync?'),
      '#type' => 'checkbox',
      '#default_value' => $has_features,
      '#access' => $has_features,
    );
  }
  $form['cache'] = array(
    '#title' => t('Clear cache on Destination after content sync?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  return $form;
}
