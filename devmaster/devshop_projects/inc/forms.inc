<?php
/**
 * @file devshop_project.form.inc DevShop Project forms related hooks and
 * support function include file.
 */

/**
 * Implements hook_form_alter().
 */
function devshop_projects_form_alter(&$form, &$form_state, $form_id){

  // Removing unneccesary fieldgroups
  if ($form_id == 'project_node_form'){
    unset($form['menu']);
    unset($form['revision_information']);
    unset($form['author']);
    unset($form['options']);
    unset($form['buttons']['delete']);
    unset($form['buttons']['preview']);
    if ($form['retry']['#value']) {
      $form['buttons']['submit']['#value'] = t('Save and Retry');
    }
  }

  // Create Project Wizard
  if ($form_id == 'devshop_project_create_step_sites' || $form_id == 'devshop_project_create_step_settings'){
    if ($form_state['project']->no_finish){
      unset($form['buttons']['return']);
    }
     if ($form_state['project']->no_next){
      unset($form['buttons']['next']);
    }
  }
  
  // On Hosting Task: Create Project form, do our own submission.
  if ($form_id == 'hosting_task_confirm_form' && $form['task']['#value'] == 'devshop-create'){
    $form['#submit'] = array('hosting_task_devshop_create_form_submit');
  }

  // If not a part of a project, bail out.
  if (empty($form['#node']->nid)){
    return;
  }

  // On Platform node forms, allow changing branch.
  if ($form_id == 'platform_node_form'){
    $node = $form['#node'];
    $project_node = node_load($node->project_nid);

    $branch_options = array_combine($project_node->git_branches, $project_node->git_branches);
    $form['git_branch'] = array(
      '#type' => 'select',
      '#title' => t('Branch'),
      '#default_value' => isset($node->git_branch)? $node->git_branch: NULL,
      '#options' => $branch_options,
      '#description' => t('Choose the branch that this platform should track.'),
    );
  }

  // Save values that need to be saved.
  if ($form_id == 'platform_node_form' || $form_id == 'site_node_form'){
    $form['project'] = array(
      '#type' => 'value',
      '#value' => $node->project,
    );
    $form['environment'] = array(
      '#type' => 'value',
      '#value' => $node->environment,
    );
  }
}

/**
 * Implementation of hook_form().
 */
function devshop_projects_form(&$node) {

  $form['git_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Git URL'),
    '#required' => TRUE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $node->git_url,
    '#maxlength' => 255,
    '#weight' => 1,
  );
  $form['title'] = array(
    '#type' => 'textfield',
     '#title' => t('Project Code Name'),
     '#required' => TRUE,
     '#description' => t('Choose a unique name for your project.'),
     '#size' => 40,
     '#default_value' => $node->title,
     '#maxlength' => 255,
  );
  $form['code_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Code path'),
    '#description' => t('The absolute path on the filesystem that will be used to create the platform in the directory specified above.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $node->code_path,
    '#maxlength' => 255,
    '#weight' => 2,
  );
  $form['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Domain'),
    '#description' => t('The domain name all sites will be built under.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $node->base_url,
    '#maxlength' => 255,
    '#weight' => 2,
  );

  // Don't allow editing
  if ($node->nid) {
    $locked = array('title', 'git_url', 'code_path', 'base_url');
    foreach ($locked as $field){
      $form[$field]['#value'] = $form[$field]['#default_value'];
      $form[$field]['#type'] = 'value';

      // Be nice and show it.
      $form["{$field}_display"] = $form[$field];
      $form["{$field}_display"]['#type'] = 'item';
    }
  }
  return $form;
}

/**
 * Implementation of hook_validate().
 */
function devshop_projects_validate($node, &$form) {

  // It may look find of strange that we are doing a node load when the
  // exact node we want is passed in here. But if this is an edit of an
  // existing node, not all of the node fields (like project_status) have
  // been initialized yet.

  $node = node_load($node->nid);

  // No validation if op == Delete or project_status == site_ready. The
  // latter case happens when the user is editing the tests to run fied.
  if ($node->op == t('Delete') || $node->project_status == 'sites_ready') {
    return;
  }


  // Full validation on when a creating a new node
  $add = (arg(1) == 'add' ? TRUE : FALSE);

  // Title (project code) must not have any spaces
  if(strpos($node->title, ' ') != FALSE) {
    form_set_error('title', t('Project code name must not contain any white spaces.'));
  }

  // The project code name must not be in the hosting_context table
  if (!$node->retry){
    $result = db_fetch_object(db_query("SELECT name, nid FROM {hosting_context} WHERE name = '%s'", $node->title));
    if ($node->nid != $result->nid){
      form_set_error('title', t('Project code name is unavailable.  Please choose another.'));
    }
  }

  // The project code name must be unique
  if (!$node->retry && ($result = db_fetch_object(db_query("SELECT title FROM {node} WHERE title = '%s' AND type = 'devshop_project' AND	nid <> %d", $node->title, $node->nid)))) {
    form_set_error('title', t('Project code name is already is use by another project'));
  }

  // Make sure the path is unique.
  $cp = hosting_path_normalize($node->code_path);
  if (!$node->retry && $add && $result = db_fetch_object(db_query("SELECT code_path FROM {hosting_devshop_project} WHERE code_path = '%s' AND nid <> %d", $cp, $node->nid))) {
    form_set_error('code_path', t('Code path is already in use by another project'));
  }

  // Directory must not exist
  if (!$node->retry && $add && file_exists($cp)) {
    form_set_error('code_path', t('Code path directory already exists.'));
  }
}

/**
 *  Form for platform creation
 */
function devshop_projects_platform_create_form($form_state, $project_nid) {
  $project_node = node_load($project_nid);

  $form = array();

  //Bail if no platforms yet.
  if (!isset($project_node->project_objects['platform'])){
    $retry = devshop_project_project_create_failed($project_nid, $task);
    if ($retry) {
      $form['note'] = array(
	'#type' => 'item',
	'#title' => t('NOTICE!'),
	'#description' => t('Project Create failed! You can ' . l('view the task log here.', "node/$task->nid") . ' Please ' . l('edit the project settings', "node/$project_nid/edit") . ' and make any necessary corrections and resubmit the form to try again.'),
	'#weight' => -1,
      );
    }
    else {
      $form['note'] = array(
        '#type' => 'item',
	'#title' => t('Clone & Verify'),
	'#value' => t('Your code is on its way to the server.  Once the platforms verify, you can choose an installation profile.'),
        );
    }
    return $form;
  }

  $form['platform_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Platform Name'),
    '#required' => TRUE,
    '#description' => t('Enter the name of the platform to be added to project ' . $project_node->title . '. Once the platform is created and verified, a site will be automatically created and added to this project.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => '',
    '#maxlength' => 255,
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create platform'),
    '#weight' => 32,
  );
  return $form;
}

/**
 * Validates for platform create
 */
function devshop_projects_platform_create_form_validate(&$form, &$form_state){
  if (empty($form_state['values']['platform_name'])){
    form_set_error('install_profile', t('You must include a platform name'));
  }


}

/**
 * Submit for platform create
 *
 */
function devshop_projects_platform_create_form_submit(&$form, &$form_state){

  $project_node = node_load($form_state['values']['nid']);
  $args = array();
  if ($form_state['values']['git_url']) {
    $args['git-url'] = $form_state['values']['git_url'];
  }

  if ($form_state['values']['branch']) {
    $args['branch'] = $form_state['values']['branch'];
  }

  $args['platform-name'] = $form_state['values']['platform_name'];

  hosting_add_task($project_node->nid, 'devshop-platform-create', $args);

  $form_state['redirect'] = "node/{$project_node->nid}";
  drupal_goto("node/{$project_node->nid}");
}

/**
 *  Form for project delete
 */
function devshop_projects_project_delete_form($form_state, $project_nid) {
  $project_node = node_load($project_nid);

  $form = devshop_projects_view($project_node)->content;

  unset($form['sites']['add_platform']);
  unset($form['tasks_view']);
  unset($form['devshop']);
  unset($form['info']['#prefix']);
  unset($form['info']['#suffix']);

  $form['remove_dir'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove all project files and directories.'),
    '#description' => t("Note that the platform & site directories are removed regardless of this option. By choosing this option, everything else in the directory '{$project_node->code_path}', including the directory itself, will be removed."),
    '#default_value' => FALSE,
    '#weight' => 30,
  );

  $form['message'] = array(
    '#type' => 'item',
    '#value' => t("Are you sure you want to delete project '{$project_node->title}' and all of its associated sites and platforms?"),
    '#weight' => 31,
  );

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_nid,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Project'),
    '#weight' => 32,
  );

  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#weight' => 33,
  );

  return $form;
}

/**
 *  Form for project delete submit handler
 */
function devshop_projects_project_delete_form_submit(&$form, &$form_state){

  // Make sure the user really wants to delete the project
  if ($form_state['clicked_button']['#value'] == t('Delete Project')) {
    $nid = $form['nid']['#value'];
    $data = devshop_projects_project_data_get($nid);

    // First get a list of all of the objects for this project
    $query = db_query("SELECT * " .
		      "FROM {hosting_devshop_project_object} " .
		      "WHERE project_nid = %d", $nid);

    $snid = array();
    $pnid = array();

    while ($obj = db_fetch_object($query)) {
      if ($obj->object_type == 'site') {
	$snid[] = $obj->object_nid;
      }
      else if ($obj->object_type == 'platform') {
	$pnid[] = $obj->object_nid;
      }
    }

    if (count($snid) > 0) {
      $data['project_delete_site_disable'] = $snid;
      $data['project_delete_site_delete'] = $snid;
    }
    else {
      unset($data['project_delete_site_disable']);
      unset($data['project_delete_site_delete']);
    }

    if (count($pnid) > 0) {
      $data['project_delete_platform_delete'] = $pnid;
    }
    else {
      unset($data['project_delete_platform_delete']);
    }

    if ($data['project_delete_platform_delete'] ||
	$data['project_delete_site_disable']   ||
	$data['project_delete_site_delete'] ) {
      $data['deleting_project'] = TRUE;
      if($form_state['values']['remove_dir']) {
	$data['deleting_project_remove_dir'] = TRUE;
      }
    }
    else {
      unset($data['deleting_project']);
    }

    // Save the delete task and nid lists. We're going to need them
    // when the hosting post task hook gets invoked
    devshop_projects_project_data_set($nid, $data);

    // Kick the whole thing off by deleting the project first
    hosting_add_task($nid, 'delete');
  }

  // Go back to the main project list page
  $form_state['redirect'] = 'hosting/projects';
  drupal_goto('hosting/projects');
}




