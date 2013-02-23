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
    global $user;

    $project = $form['#node'];

    unset($form['menu']);
    unset($form['revision_information']);
    unset($form['author']);
    unset($form['options']);
    unset($form['buttons']['delete']);
    unset($form['buttons']['preview']);
    if ($form['retry']['#value']) {
      $form['buttons']['submit']['#value'] = t('Save and Retry');
    }

    //Add button for delete project
    $form['buttons']['delete'] = array(
      '#type' => 'markup',
      '#value' => l(t('Delete this project'), 'node/'. $project->nid .'/project_delete', array('query' => array('token' => drupal_get_token($user->uid)))),
      '#weight' => 10,
    );
  }

  // Create Project Wizard
  if ($form_id == 'devshop_project_create_step_sites' || $form_id == 'devshop_project_create_step_settings' || $form_id == 'devshop_project_create_step_environments'){
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

   // On Hosting Task: Delete Project form, do our own submission.
  if ($form_id == 'hosting_task_confirm_form' && $form['task']['#value'] == 'devshop-delete') {
    $node = node_load($form['nid']['#value']);

    if ($node->type == 'project') {
      $form['#submit'] = array('hosting_task_devshop_delete_form_submit');
    }
  }


  // If not a part of a project, bail out.
  if (empty($form['#node']->nid)){
    return;
  }

  // Save values that need to be saved.
  if ($form_id == 'platform_node_form' || $form_id == 'site_node_form'){
    $node = $form['#node'];
    $form['project'] = array(
      '#type' => 'value',
      '#value' => $node->project,
    );
    $form['environment'] = array(
      '#type' => 'value',
      '#value' => $node->environment,
    );
    $form['git_branch'] = array(
      '#type' => 'value',
      '#value' => $node->git_branch,
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
  
  // Unchanging Values
  $form['git_branches'] = array(
    '#type' => 'value',
    '#value' => $node->git_branches,
  );
  $form['git_tags'] = array(
    '#type' => 'value',
    '#value' => $node->git_tags,
  );
  
  // Extensible settings
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Environment Settings'),
    '#theme' => 'devshop_projects_settings_form',
    '#tree' => TRUE,
  );
  
  $settings = module_invoke_all('devshop_project_settings', $node);
  foreach ($node->project_objects['platform'] as $nid => $environment_name){
    $platform = node_load($nid);
    $form['settings'][$environment_name] = array(
      '#tree' => TRUE,
      '#title' => $name,
      '#type' => 'fieldset',
      '#theme' => 'devshop_projects_settings_table',
    );
    //$form['settings'][$name]['git_branch'] = array(
    //  '#type' => 'select',
    //  '#title' => t('Branch'),
    //  '#default_value' => isset($platform->git_branch)? $platform->git_branch: NULL,
    //  '#options' => $branch_options,
    //  '#description' => t('Choose the branch that this platform should track.'),
    //);
    foreach ($settings as $setting_id => $setting){
      $form['settings'][$environment_name][$setting_id] = $setting;
      $form['settings'][$environment_name][$setting_id]['#default_value'] = $node->settings[$environment_name][$setting_id];
    }
  }
  $form['#submit'] = array('devshop_projects_submit_settings');


  // Don't allow editing
  if ($node->nid) {
    $locked = array('title', 'git_url', 'code_path', 'base_url');
    foreach ($locked as $field){
      $form[$field]['#value'] = $form[$field]['#default_value'];
      $form[$field]['#type'] = 'value';
    }
  }
  return $form;
}

/**
 * Submit function for save data of platforms.
 */
function devshop_projects_submit_settings($form, &$form_state) {
  

  // Go through and save our settings
  $project_node = node_load($form_state['values']['nid']);
  $settings_info = module_invoke_all('devshop_project_settings', $project_node);
  $nodes = array();
  
  // Go through each environment
  foreach ($form_state['values']['settings'] as $environment_name => $settings){
    
    // Then through each setting of each environment
    foreach ($settings as $setting_name => $setting_value){
      
      //Find the node type for this setting.
      $node_type = $settings_info[$setting_name]['#node_type'];
      $nids = array_flip($project_node->project_objects[$node_type]);
      
      // Load the site or platform node for this environment, then set the value and save.
      $nid = $nids[$environment_name];
      if (empty($nodes[$nid])){
        $nodes[$nid] = node_load($nid);
        $nodes[$nid]->no_verify = TRUE;
      }
      $nodes[$nid]->{$setting_name} = $setting_value;
    }
  }
  
  // Go save all nodes
  foreach ($nodes as $nid => $node){
    node_save($node);
  }
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
