<?php

function devshop_projects_create_wizard($step = NULL){
  // required includes for wizard
  ctools_include('wizard');
  ctools_include('object-cache');
  
  $form_info = array(
    'id' => 'devshop_project_create',
    'path' => "node/add/project/%step",
    'show trail' => TRUE,
    'show back' => TRUE,
    'show cancel' => TRUE,
    'show return' => FALSE,
    'next text' => 'Next',
    'next callback' =>  'devshop_projects_create_wizard_next',
    'finish callback' => 'wombat_basic_add_subtask_finish',
    'return callback' => 'wombat_basic_add_subtask_finish',
    'cancel callback' => 'wombat_basic_add_subtask_cancel',
    'order' => array(
      'git_url' => t('Step 1: Get Code'),
      'environments' => t('Step 2: Choose Branches'),
      'install' => t('Step 2: Install Drupal'),
    ),
    'forms' => array(
      'git_url' => array(
        'form id' => 'devshop_project_create_step_1_git'
      ),
      'environments' => array(
        'form id' => 'devshop_project_create_step_2_environments'
      ),
      'install' => array(
        'form id' => 'devshop_project_create_step_3_install'
      ),
    ),
  );
  
  // *** SETTING THE FORM UP FOR MULTISTEP *** //
  $form_state = array(
    'cache name' => NULL,
  );
  $project = devshop_project_create_get_cache(NULL);
  if ($step == NULL){
    $step = 'git_url';
  }
  if (empty($project)){
    // set form to first step -- we have no data
    $step = current(array_keys($form_info['order']));
    $project = new stdClass();

    // ** set the storage object so its ready for whatever comes next
    ctools_object_cache_set('project', $form_state['cache name'], $project);
  }
  
  //THIS IS WHERE WILL STORE ALL FORM DATA
  $form_state['project'] = $project;
  
  // and this is the witchcraft that makes it work
  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
  return $output;
}


/**
 * STEP 1: Form
 *
 * Just get the Git URL.  If we can't access the code, all else should fail.
 */
function devshop_project_create_step_1_git(&$form, &$form_state) {
  $project = &$form_state['project'];
  $form['git_url'] = array(
    '#type' => 'textfield',
    '#required' => 1,
    '#title' => t('Git URL'),
    '#description' => t('Enter the Git URL for your drupal project.  The root of the repo must contain Drupal\'s index.php.  A clone of !link is a good place to start.', array('!link' => l('http://git.drupal.org/project/drupal.git', 'http://git.drupal.org/project/drupal.git'))),
    '#default_value' => $project->git_url,
  );
}

/**
 * STEP 1: Validator.
 *
 * This is where we try and connect to the remote branches.
 *
 * As long as the SSH for www-data is setup right, this will work for private repos.
 *  @TODO: Figure out any security implications of giving www-data an ssh private key and telling users that
 *  (if they want web-based validation) they have to add www-data's SSH key to the repo as well.
 *
 *  This is also where we get all the tags and branches.
 */
function devshop_project_create_step_1_git_validate(&$from, &$form_state) {
  $project = &$form_state['project'];
  $project->git_url = $form_state['values']['git_url'];
  
  $command = "git ls-remote {$project->git_url}";
  $remote_list = explode("\n", shell_exec($command));
  
  // If remote list is empty, we have either a typo or a connection error.
  if (count($remote_list) == 1 && empty($remote_list[0])){
    form_set_error('git_url', t('This server was not able to connect to that Git URL.  Please check the URL, and make sure www-data and aegir user has SSH keys setup to connect to your repos.'));
    return;
  }
  
  // Build tag and branch list
  $project->branches = array();
  $project->tags= array();
  
  foreach ($remote_list AS $line_string){
    // @TODO: Would love some regex love here
    // Example remote line:
    // 9fc5727c0823d8d3300ba5aae3328d5998033e45	refs/heads/3-00
    $line = explode("	", $line_string);
    $ref = explode("/", $line[1]);
    $branch = array_pop($ref);
    
    if ($ref[1] == 'heads') {
      $project->branches[] = $branch;
    } else if ($ref[1] == 'tags') {
      $project->tags[] = $branch;
    }
  }
}

/**
 * KEY CONCEPT: generally, you will never save data here -- you will simply add values to the 
 * yet to be saved ctools_cache object. 
 * 
 * Saving happens at the end, within the $form_info['finish callback'];
 */
function devshop_project_create_step_1_git_submit(&$from, &$form_state) {
  $project = &$form_state['project'];
  $project->git_url = $form_state['values']['git_url'];
}

/**
 * STEP 2: Choose environments
 */
function devshop_project_create_step_2_environments(&$form, &$form_state) {
  $project = &$form_state['project'];
  dsm($project);
  $default_platforms = variable_get('devshop_default_platforms', array('dev', 'test', 'live'));
  
  $branch_options = array(t('Choose a branch...'));
  $branch_options += array_combine($project->branches, $project->branches);
  
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
      '#default_value' => isset($project->default_platforms[$env]),
    );
    $form['default_platforms'][$env]['branch'] = array(
        '#type' => 'select',
        '#default_value' => $project->default_platforms[$env],
        '#options' => $branch_options
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
 * STEP 2: SUBMIT
 */
function devshop_project_create_step_2_environments_validate(&$form, &$form_state) {
  $project = &$form_state['project'];
  $values = $form_state['values'];
  
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
 * STEP 2: SUBMIT
 */
function devshop_project_create_step_2_environments_submit(&$form, &$form_state) {
  // Get project and reset properties..  
  $project = &$form_state['project'];
  $values = $form_state['values'];

  // Set Values
  $project->default_platforms = $values['default_platforms'];
  $project->branch_platforms = $values['branch_platforms'];
}



/**
 * NEXT callback
 * Saves anything in $form_state['project'] to ctools cache.
 *
 * The form submit callback is responsible for putting data into $form_state['project'].
 */
function devshop_projects_create_wizard_next(&$form_state) {
  $project = &$form_state['project'];
  $cache = ctools_object_cache_set('project', $form_state['cache name'], $project);
}


/*----PART 3 CTOOLS CALLBACKS -- these usually don't have to be very unique  ---------------------- */

/**
 * Callback generated when the add page process is finished.
 * this is where you'd normally save.
 */
function wombat_basic_add_subtask_finish(&$form_state) {
  $wombat = &$form_state['wombat_obj'];
  drupal_set_message('Wombat '.$wombat->name.' successfully deployed. Wombat will be assigned the following duties: '.implode(",", $wombat->duties)).'.';
  // Clear the cache
  ctools_object_cache_clear('wombat_basic', $form_state['cache name']);
  $form_state['redirect'] = 'wombat';
}


/**
 * Callback generated when the 'cancel' button is clicked.
 *
 * All we do here is clear the cache.
 */
function wombat_basic_add_subtask_cancel(&$form_state) {
  // Update the cache with changes.
  ctools_object_cache_clear('wombat_basic', $form_state['cache name']);
  $form_state['redirect'] = 'wombat';
  drupal_set_message('Coward.');
}

/*----PART 4 CTOOLS FORM STORAGE HANDLERS -- these usually don't have to be very unique  ---------------------- */

/**
 * Remove an item from the object cache.
 */
function  devshop_project_create_clear_cache($name) {
  ctools_object_cache_clear('project', $name);
}

/**
 * Get the cached changes to a given task handler.
 */
function devshop_project_create_get_cache($name) {
  $cache = ctools_object_cache_get('project', $name);
  return $cache;
}




function d($in) {
  print '<pre>';
  print_r($in);
  print '</pre>'; 
}