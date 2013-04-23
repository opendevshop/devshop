<?php

/**
 * Project Create Wizard Page
 */
function devshop_projects_create_wizard($step = NULL){
  // required includes for wizard
  ctools_include('wizard');
  ctools_include('object-cache');
  drupal_add_css(drupal_get_path('module', 'devshop_projects') . '/inc/devshop.css');

  // Setup form info
  $form_info = devshop_projects_create_wizard_info();
  $form_state = array(
    'cache name' => NULL,
  );
  
  // Skip step 2 if needed
  $skip = variable_get('devshop_projects_skip_settings', TRUE);
  if ($skip) {
    $form_info['order'] = array(
      'git_url' => t('Step 1: Source'),
      'environments' => t('Step 2: Environments'),
      'sites' => t('Step 3: Install Profile'),
    );
  }

  // Setup project
  $project = ctools_object_cache_get('project', NULL);

  // Setup Step.
  if ($step == NULL){
    drupal_goto('hosting/projects/add/' . current(array_keys($form_info['order'])));
  }

  // Create default project object
  if (empty($project)){
    // set form to first step -- we have no data
    $step = current(array_keys($form_info['order']));
    $project = new stdClass();
    $project->step = $step;
    $project->git_url = '';
    $project->project_nid = NULL;
    $project->title = '';
    $project->environments = array('NEW' => array());
    $project->live_domain = '';

    // ** set the storage object so its ready for whatever comes next
    ctools_object_cache_set('project', $form_state['cache name'], $project);
  } else {
    // Quickly save the current step
    $project->step = $step;
    ctools_object_cache_set('project', $form_state['cache name'], $project);
  }
  
  // Check verification status
  $project_node = node_load($project->project_nid);
  if (!empty($project_node->nid)){
    $tasks = hosting_task_fetch_tasks($project_node->nid);
  }
  if (isset($tasks['verify']['nid'])){
    
  }
  $project->verify_task_status = isset($tasks['verify']['task_status'])? $tasks['verify']['task_status']: HOSTING_TASK_ERROR;
  $project->verify_task_nid = $tasks['verify']['nid'];
  
  // If project verification failed, we need to ask for a new git url.
  if ($project->verify_task_status === HOSTING_TASK_ERROR && !empty($project_node->nid)){
    drupal_set_message(t('Unable to connect to the git url: !git.  Please check the git URL and try again.', array('!git' => $project->git_url)));
    // If not on the first step, go to it.
    if ($step != current(array_keys($form_info['order']))){
      drupal_goto('hosting/projects/add/' . current(array_keys($form_info['order'])));
    }
  }

  // All forms can access $form_state['project'];
  $form_state['project'] = $project;

  // Saving the last visited step for redirects from node
  $_SESSION['last_step'] = $step;

  // Generate our ctools form and output
  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
  return $output;
}

/**
 * The form_info for the ctools wizard
 */
function devshop_projects_create_wizard_info(){
  return array(
    'id' => 'devshop_project_create',
    'path' => "hosting/projects/add/%step",
    'show trail' => TRUE,
    'show back' => TRUE,
    'show cancel' => TRUE,
    'show return' => FALSE,
    'next text' => 'Next',
    'next callback' =>  'devshop_projects_create_wizard_next',
    'finish callback' => 'devshop_projects_create_wizard_finish',
    'cancel callback' => 'devshop_projects_create_wizard_cancel',
    'order' => array(
      'git_url' => t('Step 1: Source'),
      'settings' => t('Step 2: Settings'),
      'environments' => t('Step 3: Environments'),
      'sites' => t('Step 4: Install Profile'),
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
      'sites' => array(
        'form id' => 'devshop_project_create_step_sites'
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
  global $user;

  $project = &$form_state['project'];

  // Save the extra options to the project node.
  $node = node_load($project->project_nid);
  $node->code_path = $project->code_path;
  $node->base_url = $project->base_url;
  $node->install_profile = $project->install_profile;
  $node->live_domain = $project->live_domain;
  $node->drupal_path = $project->drupal_path;
  $node->uid = $user->uid;
  $node->status = 1;
  node_save($node);

  // Create the site nodes
  // @TODO: Can we speed things up here by only running install for the first,
  // then  "Cloning" to create the rest?
  foreach ($node->project_objects['platform'] as $nid => $env) {
    $db_server = $project->environments[$env]['db_server'];
    devshop_projects_create_site($node, node_load($nid), $env, $db_server);
  }

  ctools_object_cache_clear('project', $form_state['cache name']);
  $form_state['redirect'] = 'node/' . $node->nid;

  // Removing last step session variable.
  unset($_SESSION['last_step']);
  
  // Friendly message
  drupal_set_message(t('Your project has been created.  Once installed, your sites will be available.'));
}


/**
 * CANCEL callback
 * Callback generated when the 'cancel' button is clicked.
 * Remove the project data cache and send back to projects page.
 */
function devshop_projects_create_wizard_cancel(&$form_state) {
  // Update the cache with changes.
  $project = &$form_state['project'];
  ctools_object_cache_clear('project', $form_state['cache name']);

  // Redirect to projects list
  $form_state['redirect'] = 'hosting/projects';

  // If we have a project node, create a "delete" hosting task
  if (!empty($project->project_nid)){
    hosting_add_task($project->project_nid, 'delete');
  }

  // Removing last step session variable.
  unset($_SESSION['last_step']);

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
    '#description' => t('Choose a unique name for your project.  For consistency, its a good idea to use the name of your git repository. <em>NOTE: You cannot change this, but you can delete and start over.Choose wisely.</em>'),
    '#size' => 40,
    '#default_value' => $project->title,
    '#maxlength' => 255,
  );

  // If there is already a title, make this an item.
  if (!empty($project->title)) {
    // Force title
    $form['title']['#value'] = $form['title']['#default_value'];
    $form['title']['#type'] = 'value';
    $form['title']['#description'] = t('You cannot change the name of your project once it has been created.  If you must, click the "Cancel" button to delete this project, then create a new one.');

    // Be nice and show it.
    $form['title_display'] = $form['title'];
    $form['title_display']['#type'] = 'item';
  }
  
  // Display helpful tips for connecting.
  $pubkey = variable_get('devshop_public_key', '');
  
  // If we don't yet have the server's public key saved as a variable...
  if (empty($pubkey)){
    $output = t("For convenience, the server's SSH public key will be displayed here, once you run the following command on your server:");
    $command = 'drush @hostmaster vset devshop_public_key "$(cat ~/.ssh/id_rsa.pub)" --yes';
    $output .= "<div class='command'><input size='160' value='$command' onclick='this.select()' /></div>";
  } else {
    // @TODO: Make this Translatable
    $output = <<<HTML
  <div class="empty-message">If you haven't granted this server access to your Git repository, you should do so now using it's <a href="#rsa" title="Click to see this server's public SSH key" onclick="$('#rsa').toggle().select()">public SSH key</a>.</div>
  <textarea id="rsa" onclick="this.select()">$pubkey</textarea>
HTML;
  }

  // Add info about connecting to Repo
  $form['connect'] = array(
    '#type' => 'item',
    '#title' => t('Repository Access'),
    '#description' => $output,
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

  // No spaces or special characters allowed.
  $url = strtolower(trim($form_state['values']['title'])); // domain names are case-insensitive
  if (!_hosting_valid_fqdn($url)) {
    form_set_error('title', t("You have not specified a valid project codename.  Only numbers and letters are allowed."));
  }

  // Check for duplicate project name here.
  $node = hosting_context_load($form_state['values']['title']);
  if ($node->nid != $project->project_nid){
    form_set_error('title', t('That name is in use.  Please try again.'));
  }
}

/**
 * STEP 1: Submit
 */
function devshop_project_create_step_git_submit(&$from, &$form_state) {
  global $user;

  $project = &$form_state['project'];

  // If the project already exists, and git url has changed...
  if ($project->project_nid) {
    // Change the git url and save the node.  Verification SHOULD run again.
    $node = node_load($project->project_nid);
    $node->git_url = $form_state['values']['git_url'];
    $node->branches = array();
    node_save($node);

    // Also clear the repo related info from $project
  }

  // Now that we've compared old and new, set $project properties.
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
    $node->status = 0;
    $node->uid = $user->uid;
    $node->name = $user->name;
    if ($node = node_submit($node)) {
      node_save($node);
    }
    if ($node->nid){
      $project->project_nid = $node->nid;
    }
  }

  // Now that we have the project name, set the defaults for code path and project URL
  $project->code_path = variable_get('devshop_projects_path', '/var/aegir/projects'). '/'. $project_name;

  // Setup project url.
  $base_url = $_SERVER['SERVER_NAME'];
  $server = variable_get('devshop_project_master_url', $base_url);
  $project->base_url = strtr(variable_get('devshop_project_default_base_url_pattern', "@project.@hostname"), array('@project' => strtolower($project_name), '@hostname' => $server));
  
  
  //Verify if is need to skip.
  $skip = variable_get('devshop_projects_skip_settings', TRUE);
  if ($skip) {
    $project->code_path = variable_get('devshop_projects_default_path', '/var/aegir/projects');
    $project->drupal_path = variable_get('devshop_projects_default_drupal_path', '');
    $project->step = 'environments';
  }
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
    '#description' => t('The absolute path on the filesystem that will be used to create all platforms within this project.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->code_path,
    '#maxlength' => 255,
  );
  $form['drupal_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Drupal'),
    '#description' => t('Enter the relative path to the index.php file in your repository. Leave blank if index.php is in the root.'),
    '#size' => 40,
    '#default_Value' => $project->drupal_path,
    '#maxlength' => 255,
  );
  $form['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Domain'),
    '#description' => t('All sites will be under a subdomain of this domain.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->base_url,
    '#maxlength' => 255,
  );
}

/**
 * STEP 2: Validate
 */
function devshop_project_create_step_settings_validate(&$from, &$form_state) {
  $project = &$form_state['project'];

  // Code path and domain must be unique
  if (db_result(db_query('SELECT n.nid FROM {hosting_devshop_project} d LEFT JOIN {node} n ON d.nid = n.nid WHERE status = 1 AND code_path = "%s";', $form_state['values']['code_path']))){
    form_set_error('code_path', t('Another project already has this code path.  Code path must be unique.'));
  }
  if (db_result(db_query('SELECT n.nid FROM {hosting_devshop_project} d LEFT JOIN {node} n ON d.nid = n.nid WHERE status = 1 AND base_url = "%s";', $form_state['values']['base_url']))){
    form_set_error('base_url', t('Another project already has this base url.  Base URL must be unique.'));
  }

}

/**
 * STEP 2: Submit
 */
function devshop_project_create_step_settings_submit(&$from, &$form_state) {
  $project = &$form_state['project'];

  // For this step, we just save code path and base url for later saving and
  // verification.
  $project->code_path = $form_state['values']['code_path'];
  $project->drupal_path = $form_state['values']['drupal_path'];
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
  $project_node = node_load($project->project_nid);

  if ($project->verify_task_status == HOSTING_TASK_QUEUED || $project->verify_task_status == HOSTING_TASK_PROCESSING) {
    // @TODO: Detect and show errors when they occur.
    // @TODO: Pretty this up, figure out how to maybe, display the task in the body?
    $note = '<p class="wait">' . t('Please wait while we connect to your repository and determine any branches.') . '</p>';
    
    $form['note'] = array(
      '#type' => 'markup',
      '#value' => $note,
    );
    $form['not_ready'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
    $project->no_next = TRUE;

    // Add code to reload the page when complete.
    devshop_form_reloader($form, 'project');
    return;
  }
  // Handle failure
  elseif ($project->verify_task_status == HOSTING_TASK_ERROR) {
    $type = 'error';
    $error = db_result(db_query("SELECT message FROM {hosting_task_log} WHERE nid = %d AND type = '%s' ORDER BY vid, lid", $project->verify_task_nid, $type));  
    drupal_set_message($error, 'error');
    $note = '<p class="wait">'. t('Something Failed.  Please check the error message, and retry verification.') . '</p>';
    
    $form['note'] = array(
      '#type' => 'markup',
      '#value' => $note,
    );
    
    $form['not_ready'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
     return;
  }
  
  // this JS handles the form element hiding/showing
  $path = drupal_get_path('module', 'devshop_projects') . '/inc/environments.js';
  drupal_add_js($path);

  $settings = module_invoke_all('devshop_project_settings', $project_node);
  
  $form['environments'] = array(
    '#type' => 'fieldset',
    '#theme' => 'devshop_projects_create_settings_form',
    '#tree' => TRUE,
    '#prefix' => '<div id="environments-wrapper">',
    '#suffix' => '</div>',
  );

  foreach ($project->environments as $env => $env_settings) {
    // No platforms exist yet
    if ($env == 'NEW') {
      $env_title = '';
    } else {
      $env_title = $env;
    }
    $form['environments'][$env] = array(
      '#tree' => TRUE,
      '#type' => 'fieldset',
      '#theme' => 'devshop_projects_settings_table',
    );

    $form['environments'][$env]['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Environment Name'),
      '#default_value' => $env_title,
      '#size' => 25,
      '#maxlength' => 255,
      '#attributes' => array(
        'placeholder' => t('Name this environment...'),
      ),
    );

    // Add environment settings form elements
    foreach ($settings as $setting_id => $setting){
      $form['environments'][$env][$setting_id] = $setting;
      $form['environments'][$env][$setting_id]['#default_value'] = $project->environments[$env][$setting_id];      
    }

    //Now add button.
    $form['add_environment'] = array(
      '#type' => 'submit',
      '#value' => t('Add environment'), 
      '#name' => 'add_environment',
      '#submit' => array('devshop_projects_create_wizard_add_new_environment'),
    ); 
  }
}


/**
 * STEP 3: Validate
 */
function devshop_project_create_step_environments_validate(&$form, &$form_state) {
  $project = &$form_state['project'];
  $values = &$form_state['values'];

  // Project not verified yet.  We don't know the branches.
  if (isset($values['not_ready'])) {
    form_set_error('', t('We haven\'t connected to your repo yet.  Please wait for the queue.'));
    return;
  }
  
  // Check environment titles
  foreach ($values['environments'] as $env => $env_settings) {
    // Check for illegal chars
    if ($env != 'NEW' && !preg_match('!^[a-z0-9_]+$!', $env_settings['title'])) {
      $form_item = 'environments][' . $env  . '][title';
      form_set_error($form_item, t('The environment name must contain only lowercase letters, numbers, and underscores.'));
    }
  }
  
  // Change NEW environment data to $title
  if ($values['environments']['NEW']['title']){
    $new_env = $values['environments']['NEW']['title'];
    $new_env_settings = $values['environments']['NEW'];
    $values['environments'][$new_env] = $new_env_settings;

    // Create the next NEW environment
    unset($values['environments']['NEW']);
    $values['environments']['NEW'] = array();
  }
  
  // Save envs to project
  $project->environments = $values['environments'];
  
  // Reject if empty
  if (count($project->environments) - 1 < 1){
    form_set_error('environments][NEW][title', t('You must add at least one environment.'));
  }
}

/**
 * STEP 3: SUBMIT
 */
function devshop_project_create_step_environments_submit(&$form, &$form_state) {
  // Get project and reset properties..
  $project = &$form_state['project'];
  $values = $form_state['values'];

  $project->environments = array_filter($values['environments']);

  $first_envs = $project->environments ? $project->environments : array();
  $settings = module_invoke_all('devshop_project_settings', $project_node);
  
  // Create these platforms
  foreach ($project->environments as $name => $data) {
    
    // If platform hasn't been created yet, do so now!
    if (empty($data['platform_nid'])) {
      // Save the damn platform nodes
      $platform = new stdClass;

      // Platform name
      $platform->title = $project->title . '_' . $data['title'];
      
      // Platform publish_path
      if ($project->drupal_path) {
        $platform->publish_path = $project->code_path . '/' . $data['title'] . '/' . $project->drupal_path;
      }
      else {
        $platform->publish_path = $project->code_path . '/' . $data['title'];
      }

      // Other attributes
      $platform->web_server = $data['web_server'];
      $platform->git_branch = $data['git_branch'];
      $platform->pull_enable = $data['pull_enabled'];
      $platform->project = $project->title;
      $platform->environment = $data['title'];
      $platform->drupal_path = $project->drupal_path;

      $platform_node = _devshop_projects_node_create('platform', $platform);
      $project->environments[$name]['platform_nid']  = $platform_node->nid;
    }
    // If the platform already exists, save it again with new settings.
    else {
      $platform_node = node_load($project->platforms[$data['title']]);
      foreach ($settings as $setting_name => $element){
        $platform_node->{$setting_name} = $data[$setting_name];
      }
      node_save($platform_node);
    }
  }

  // For all removed platforms, trigger a delete task
  $removed_environments  = array_diff_key($first_envs, $values['environments']);
  foreach ($removed_environments as $environment_name => $settings) {

    // @TODO: Determine what to do here based on task status...
    // if verify task hasn't even run yet (and has never run) we can just delete
    // the platform node.

    // Create 'delete' task for this project
    hosting_add_task($project->environments[$environment_name]['platform_nid'], 'delete');

    // Remove from project object
    unset($project->environments[$environment_name]);
  }
}

/**********
 * STEP 4
 * Project Settings
 *********/

/**
 * STEP 4: Form
 */
function devshop_project_create_step_sites(&$form, &$form_state) {
  $project = &$form_state['project'];
  $project_node = node_load($project->project_nid);

  // OUTPUT
  if ($project_node->project_status == 'sites_installing') {
    $form['note'] = array(
      '#type' => 'item',
      '#title' => t('Sites Installing'),
      '#value' => t('Your sites are being installed!'),
    );
    return $form;
  }
  elseif ($project_node->project_status == 'preparing-project') {
    // @TODO: Not yet, needs more logic
    // drupal_goto('hosting/projects/add');
  }
  elseif ($project_node->project_status == 'sites-installed') {
    return;
  }

  $profiles = array();
  $available_profiles = array();
  $completed = TRUE;

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,
  );

  // Display the platforms
  $rows = array();
  $header = array(t('Name'), t('Branch'), t('Version'), t('Verification Status'), t('Installation Profiles'));
  foreach ($project_node->project_objects['platform'] as $nid => $name){

    // If this platform is no longer in the $project object, we shouldn't show
    // it here.
    if (!isset($project->environments[$name])) {
      continue;
    }

    // Get verification task for this platform.
    $task = hosting_get_most_recent_task($nid, 'verify');
    $platform = node_load($nid);

    $row = array();
    $row[] = $name;
    $row[] = $platform->git_branch;
    $row[] = $task->task_status == HOSTING_TASK_SUCCESS? $platform->release->version: t('Pending');
    $row[] =  _hosting_parse_error_code($task->task_status);

    // If platform verified successfully:
    if ($task->task_status == HOSTING_TASK_SUCCESS) {

      // Collect install profiles
      $profiles_shortnames = hosting_get_profiles($platform->nid, 'short_name');
      if (is_array($profiles_shortnames) && !empty($profiles_shortnames)) {
        $profiles[$name] = array_combine($profiles_shortnames, (array) hosting_get_profiles($platform->nid));
        $row[] = implode(', ', $profiles[$name]);
      } else {
        $profiles[$name] = array();
      }

      // If no available profiles found yet, this becomes the available list.
      if (empty($available_profiles)){
        $available_profiles = $profiles[$name];
      }
      // If we have available profiles, only keep the ones we found again.
      else {
        // @TODO: This joins profiles by name, not profile name.
        $available_profiles = array_intersect_key($available_profiles, $profiles[$name]);
      }
    }
    // If platform verification failed:
    elseif ($task->task_status == HOSTING_TASK_ERROR) {
      $completed = FALSE;
      $row[] = t('Platform verification failed!  Check the task log for more details. You can !link', array('!link' => l(t('Go Back'), 'hosting/projects/add/environments')));
    }
    // If platform is still processing:
    elseif ($task->task_status == HOSTING_TASK_PROCESSING || $task->task_status == HOSTING_TASK_QUEUED) {
      $completed = FALSE;
      $row[] = t('Platform download & verification in process.');
    }

    // If platform has NOT verified successfully, mark process not complete.
    else {
      $completed = FALSE;
      $row[] = t('Pending');
    }

    // Store rows for display
    $rows[] = $row;
  } // end foreach platform
  

  // Output our table.
  $form['platforms'] = array(
    '#type' => 'markup',
    '#value' => theme('table', $header, $rows),
  );
  
  // Provide something for people while they wait.
  if (!$completed){
    $project->no_finish = TRUE;
    $note = '<p class="wait">' . t('Please wait while we download and verify your drupal code.') . '</p>';
    
    devshop_form_reloader($form, 'platform');


    $form['help'] = array(
      '#type' => 'markup',
      '#value' => $note,
    );
    return $form;
  }
  // If no available profiles:
  elseif (count($available_profiles) == 0) {
    $project->no_finish = TRUE;
    $note = '<p class="wait">' . t('WARNING: No common profile was found in all of your platforms.  Please check your branches and code and try again.  You must !link to change your branches.', array('!link' => l(t('Go Back'), 'hosting/projects/add/environments'))) . '</p>';

    devshop_form_reloader($form, 'platform');

    $form['error'] = array(
      '#type' => 'markup',
      '#value' => $note,
    );
    return $form;
  } else {
   $project->no_finish = FALSE;

    // Install Profile
    // Sensible default?
    // Lets go with standard for now... we can update later.
    if (isset($available_profiles['standard'])) {
      $default_profile = 'standard';
    }
    // If 'drupal' profile exists, it is likely drupal6!
    elseif (isset($available_profiles['drupal'])) {
      $default_profile = 'drupal';
    }

    // @TODO: Handle no available profiles
    $form['install_profile'] = array(
      '#type' => 'radios',
      '#options' => $available_profiles,
      '#title' => t('Project Install Profile'),
      '#required' => 1,
      '#description' => t('All sites in your project must use the same installation profile, and it must exist in all branches.  Choose the installation profile for this project.'),
      '#default_value' => $default_profile,
    );
  }
}

/**
 * STEP 4: Validate
 */
function devshop_project_create_step_sites_validate(&$from, &$form_state) {
  $project = &$form_state['project'];

  if (empty($form_state['values']['install_profile'])){
    form_set_error('install_profile', t('You must choose an install profile'));
  }
}

/**
 * STEP 4: Submit
 *
 * Save install profile to the project object (project and site node creation
 * happens on wizard finish.)
 */
function devshop_project_create_step_sites_submit(&$from, &$form_state) {
  $project = &$form_state['project'];
  $project->install_profile = $form_state['values']['install_profile'];
}


function devshop_projects_add_status($type = 'platform'){
  $return = array();
  
  // Get Project Cache
  ctools_include('wizard');
  ctools_include('object-cache');
  $project = ctools_object_cache_get('project', NULL);
  
  
  $project_node = node_load($project->project_nid);
  
  $good_to_go = TRUE;
  $nids = array();
  
  // When checking project...
  if ($type == 'project') {
    $nids = array($project_node->nid);
  }
  
  // When checking platforms...
  if ($type == 'platform') {
    foreach ($project_node->project_objects['platform'] as $nid => $name){
      $nids[] = $nid;
    }
  }
  
  // Check verification task for all nids
  foreach ($nids as $nid){
    $task = hosting_get_most_recent_task($nid, 'verify');
    if ($task->task_status != HOSTING_TASK_SUCCESS) {
      $good_to_go = FALSE;
    }
  }
  
  $return['reload'] = $good_to_go;
  drupal_json($return);
  exit;
}

/**
 * Helper for adding reloading feature to form
 */
function devshop_form_reloader(&$form, $type = 'platform'){
  // Add JS that reloads the page when tasks finish.
  $form[$element_id] = array(
    '#type' => 'item',
    '#value' => '',
    '#prefix' => '<div id="hosting-task-list">',
    '#suffix' => '</div>',
    '#weight' => 10
  );
  $settings['devshopReload'] = array(
    'type' => $type,
    'delay' => 1000,
  );

  drupal_add_js($settings, 'setting');
  drupal_add_js(drupal_get_path('module','devshop_projects') . '/inc/reload.js');
}

/**
 * Functionality for add a new environment.
 */
function devshop_projects_create_wizard_add_new_environment($form, &$form_state) {
  // Get Project Cache
  ctools_include('wizard');
  ctools_include('object-cache');

  // All we are doing here is saving the project to cache.
  // devshop_project_create_step_environments_validate() handles putting data in $project.
  $project = &$form_state['project'];
  ctools_object_cache_set('project', NULL, $project);

  // Go back to the same page.
  drupal_goto('hosting/projects/add/environments');
}
