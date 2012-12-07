<?php

/**
 * Project Create Wizard Page
 */
function devshop_projects_create_wizard($step = NULL){
  // required includes for wizard
  ctools_include('wizard');
  ctools_include('object-cache');
  
  // Setup form info
  $form_info = devshop_projects_create_wizard_info();
  $form_state = array(
    'cache name' => NULL,
  );
  
  // Setup project
  $project = ctools_object_cache_get('project', NULL);
  
  // Setup Step.
  if ($step == NULL){
    $step = current(array_keys($form_info['order']));
  }
  
  // Create default project object
  if (empty($project)){
    // set form to first step -- we have no data
    $step = current(array_keys($form_info['order']));
    $project = new stdClass();
    $project->git_url = '';
    $project->title = '';
    $project->default_platforms = array();
    $project->branch_platforms = array();

    // ** set the storage object so its ready for whatever comes next
    ctools_object_cache_set('project', $form_state['cache name'], $project);
  }
  
  // All forms can access $form_state['project'];
  $form_state['project'] = $project;
  
  // Particular overrides
  if ($step == 'environments' && isset($project->nid)){
    $project_node = node_load($project->nid);
    if (empty($project_node->git_branches)){
      //$form_info['show back'] = FALSE;
    }
  }
  dsm($project, 'project');
  // Generate our ctools form and output
  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
  return $output;
}

/**
 * Nodemaker
 */
function _devshop_projects_node_create($type, $node = stdClass){
  global $user;
  
  // @TODO: Validate type
  $node->type = $type;
  $node->status = 1;
  $node->uid = $user->uid;
  $node->name = $user->name;
  if ($node = node_submit($node)) {
    node_save($node);
  }
  return $node;
}
/**
 * The form_info for the ctools wizard
 */
function devshop_projects_create_wizard_info(){
  return array(
    'id' => 'devshop_project_create',
    'path' => "node/add/project/%step",
    'show trail' => TRUE,
    'show back' => TRUE,
    'show cancel' => TRUE,
    'show return' => FALSE,
    'next text' => 'Next',
    'next callback' =>  'devshop_projects_create_wizard_next',
    'finish callback' => 'devshop_projects_create_wizard_finish',
    'cancel callback' => 'devshop_projects_create_wizard_cancel',
    'order' => array(
      'git_url' => t('Step 1: Git'),
      'settings' => t('Step 2: Settings'),
      'environments' => t('Step 3: Platforms'),
      'install' => t('Step 4: Install Drupal'),
    ),
    'forms' => array(
      'git_url' => array(
        'form id' => 'devshop_project_create_step_git'
      ),
      'settings' => array(
        'form id' => 'devshop_project_create_step_settings'
      ),
      'environments' => array(
        'form id' => 'devshop_project_create_step_environments'
      ),
      'install' => array(
        'form id' => 'devshop_project_create_step_install'
      ),
    ),
  );
}

/**
 * WIZARD TOOLS
 */

/**
 * NEXT callback
 * Saves anything in $form_state['project'] to ctools cache.
 *
 * The form submit callbacks are responsible for putting data into
 * $form_state['project'].
 */
function devshop_projects_create_wizard_next(&$form_state) {
  $project = &$form_state['project'];
  $cache = ctools_object_cache_set('project', $form_state['cache name'], $project);
}


/**
 * FINISH callback
 * Callback generated when the add page process is finished.
 * this is where you'd normally save.
 */
function devshop_projects_create_wizard_finish(&$form_state) {
  $project = &$form_state['project'];
  ctools_object_cache_clear('project', $form_state['cache name']);
  $form_state['redirect'] = 'hosting/projects';
}


/**
 * CANCEL callback
 * Callback generated when the 'cancel' button is clicked.
 * Remove the project data cache and send back to projects page.
 */
function devshop_projects_create_wizard_cancel(&$form_state) {
  // Update the cache with changes.
  ctools_object_cache_clear('project', $form_state['cache name']);
  $form_state['redirect'] = 'hosting/projects';
  
  // @TODO: If step 1 has been completed, we should run Delete on Project node.
  // to get rid of drush alias context, files, everything that this form
  // may have created.
}

/**
 * WIZARD STEPS
 */
 

/**********
 * STEP 1
 * Git URL
 **********/


/**
 * STEP 1: Form
 * 
 * Just get the Git URL.  If we can't access the code, all else should fail.
 */
function devshop_project_create_step_git(&$form, &$form_state) {
  $project = &$form_state['project'];
  $form['git_url'] = array(
    '#type' => 'textfield',
    '#required' => 1,
    '#title' => t('Git URL'),
    '#description' => t('Enter the Git URL for your drupal project.  The root of the repo must contain Drupal\'s index.php.  A clone of !link is a good place to start.', array('!link' => l('http://git.drupal.org/project/drupal.git', 'http://git.drupal.org/project/drupal.git'))),
    '#default_value' => $project->git_url,
  );
  $form['title'] = array(
    '#type' => 'textfield',
     '#title' => t('Project Code Name'),
     '#required' => TRUE,
     '#description' => t('Choose a unique name for your project.  For consistency, its a good idea to use the name of your git repository.'),
     '#size' => 40,
     '#default_value' => $project->title,
     '#maxlength' => 255,
  );
}

/**
 * STEP 1: Validate
 * 
 * This is where we try and connect to the remote branches.
 *
 * As long as the SSH for www-data is setup right, this will work for private repos.
 *  @TODO: Figure out any security implications of giving www-data an ssh private key and telling users that
 *  (if they want web-based validation) they have to add www-data's SSH key to the repo as well.
 *
 *  This is also where we get all the tags and branches.
 */
function devshop_project_create_step_git_validate(&$from, &$form_state) {
  $project = &$form_state['project'];
  // @TODO: Check for duplicate project name here.
}

/**
 * STEP 1: Submit
 */
function devshop_project_create_step_git_submit(&$from, &$form_state) {
  global $user;
  
  $project = &$form_state['project'];
  $project->git_url = $form_state['values']['git_url'];
  $project->title = $project_name = $form_state['values']['title'];

  if (empty($project->project_nid)){
    // Create the project node now.  We will update it with the chosen path.
    // This is so we can check for branches and save the hosting_context as soon
    // as possible.
    $node = new stdClass;
    $node->title = $project->title;
    $node->git_url = $project->git_url;
    $node->type = 'project';
    $node->status = 1;
    $node->uid = $user->uid;
    $node->name = $user->name;
    if ($node = node_submit($node)) {
      node_save($node);
    }
    if ($node->nid){
      $project->project_nid;
    }
  }
  
  // Now that we have the project name, set the defaults for code path and project URL
  $project->code_path = strtr(variable_get('devshop_project_default_code_path', "/var/aegir/projects/@name"), array('@name' => $project_name));
  
  // Setup project url.
  $hostmaster_site = hosting_context_load('hostmaster');
  $server = variable_get('devshop_project_default_base_url', $hostmaster_site->title);
  $project->base_url = strtr(variable_get('devshop_project_default_base_url_pattern', "@name.@server"), array('@name' => $project_name, '@server' => $server));
}

/**********
 * STEP 2
 * Project Settings
 *********/

/**
 * STEP 2: Form
 */
function devshop_project_create_step_settings(&$form, &$form_state) {
  $project = &$form_state['project'];
  $form['code_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Code path'),
    '#description' => t('The absolute path on the filesystem that will be used to create the platform in the directory specified above.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->code_path,
    '#maxlength' => 255,
  );
  $form['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Domain'),
    '#description' => t('The domain name all sites will be built under.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->base_url,
    '#maxlength' => 255,
  );
  
  // @TODO: Put Pull Code settings here
}

/**
 * STEP 2: Validate
 */
function devshop_project_create_step_settings_validate(&$from, &$form_state) {
  $project = &$form_state['project'];
}

/**
 * STEP 2: Submit
 */
function devshop_project_create_step_settings_submit(&$from, &$form_state) {
  global $user;
  $project = &$form_state['project'];
  
  // For this step, we just save code path and base url for later saving and
  // verification.
  $project->code_path = $form_state['values']['code_path'];
  $project->base_url = $form_state['values']['base_url'];
}
 
 
/**********
 * STEP 3
 * Project Environments
 *********/

/**
 * STEP 3: Form
 */
function devshop_project_create_step_environments(&$form, &$form_state) {
  $project = &$form_state['project'];
  $default_platforms = variable_get('devshop_default_platforms', array('dev', 'test', 'live'));
    
  $project_node = node_load($project->nid);
  
  if (empty($project_node->git_branches)){
    
    // @TODO: Detect and show errors when they occur.
    $form['note'] = array(
      '#type' => 'markup',
      '#value' => t('Please wait while we connect to your repository and determine any branches. For now you must reload the page to continue.'),
    );
    $form['not_ready'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
    return;
  }
  
  $branch_options = array_combine($project_node->git_branches, $project_node->git_branches);
  
  $form['default_platforms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default Platforms'),
    '#theme' => 'devshop_project_create_platforms_table',
    '#tree' => TRUE,
  );
  foreach ($default_platforms as $env) {
    $form['default_platforms'][$env] = array(
      '#tree' => TRUE,
    );
    $form['default_platforms'][$env]['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => $env,
      '#default_value' => !empty($project->default_platforms[$env]),
    );
    $form['default_platforms'][$env]['branch'] = array(
        '#type' => 'select',
        '#default_value' => isset($project->default_platforms[$env])? $project->default_platforms[$env]: NULL,
        '#options' => array(t('Choose a branch...')) + $branch_options
      );
  }
  
  $form['branch_platforms'] = array(
    '#type' => 'checkboxes',
    '#multiple' => 'true',
    '#title' => t('Branch Platforms'),
    '#description' => t('Check the branches you wish to create platforms for.'),
    '#default_value' => $project->branch_platforms,
    '#options' => $branch_options
  );
  $form_state['no buttons'] = TRUE; 
}


/**
 * STEP 3: Validate
 */
function devshop_project_create_step_environments_validate(&$form, &$form_state) {
  $project = &$form_state['project'];
  $values = $form_state['values'];
  
  // Not ready
  if (isset($values['not_ready'])) {
    form_set_error('', t('We haven\'t connected to your repo yet.  Please wait for the queue.'));
    return;
  }
  
  // At least one platform is required  
  $none_chosen = TRUE;
  foreach ($values['default_platforms'] as $env => $data) {
    if ($data['enabled']){
      $none_chosen = FALSE;
      if ($data['branch']){
        // Save the value
        form_set_value($form['default_platforms'][$env], $data['branch'], $form_state);
      } else {
        // If you chose a default platform, you must choose a branch
        form_set_error("default_platforms][$env][branch", t('You must choose a branch to create a platform.'));
      }
    } else {
      // Set a null value.  Otherwise this comes through as an array.
      form_set_value($form['default_platforms'][$env], NULL, $form_state);
    }
  }
  
  $branch_platforms = array_filter($values['branch_platforms']);
  if (!empty($branch_platforms)){
    $none_chosen = FALSE;
  }
  
  if ($none_chosen){
    form_set_error('form', t('You must choose to build at least one platform.'));
  }
  
  // Filter out branch platforms forms_api so we can keep the submit clean.
  form_set_value($form['branch_platforms'], array_filter($values['branch_platforms']), $form_state);
  
}

/**
 * STEP 3: SUBMIT
 */
function devshop_project_create_step_environments_submit(&$form, &$form_state) {
  // Get project and reset properties..  
  $project = &$form_state['project'];
  $values = $form_state['values'];

  // Get platforms from form values and save into our object
  $project->default_platforms = array_filter($values['default_platforms']);
  $project->branch_platforms = array_filter($values['branch_platforms']);
  
  // Create these platforms
  if (empty($project->platforms)){
    $all_platforms = array_merge($project->default_platforms, $project->branch_platforms);
    foreach ($all_platforms as $platform_name => $branch) {
      // Save the damn platform nodes
      $platform = new stdClass;
      $platform->title = $project->name . '_' . $platform_name;
      $platform->project = $project->name;
      $platform->git_branch = $branch;

      $platform_node = _devshop_projects_node_create('platform', $project);
      dsm($platform_node, $platform_node->title);
    }
  }
  
}


/**********
 * STEP 4
 * Project Environments
 *********/

/**
 * STEP 4: Form
 */
function devshop_project_create_step_install(&$form, &$form_state) {
  $project = &$form_state['project'];
  dsm($project);
  
}
