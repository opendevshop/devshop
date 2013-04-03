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
    $project->default_platforms = array();
    $project->branch_platforms = array();
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
  $node->uid = $user->uid;
  $node->status = 1;
  node_save($node);

  // Create the site nodes
  // @TODO: Can we speed things up here by only running install for the first,
  // then  "Cloning" to create the rest?
  foreach ($node->project_objects['platform'] as $nid => $env){
    devshop_projects_create_site($node, node_load($nid), $env);
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
  $project->code_path = strtr(variable_get('devshop_project_default_code_path', "/var/aegir/projects/@name"), array('@name' => $project_name));

  // Setup project url.
  $base_url = $_SERVER['SERVER_NAME'];
  $server = variable_get('devshop_project_default_base_url', $base_url);
  $project->base_url = strtr(variable_get('devshop_project_default_base_url_pattern', "@name.@server"), array('@name' => strtolower($project_name), '@server' => $server));
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

  $default_platforms = variable_get('devshop_default_platforms', array('dev', 'test', 'live'));
  if ($project->verify_task_status == HOSTING_TASK_QUEUED || $project->verify_task_status == HOSTING_TASK_PROCESSING){

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
  elseif ($project->verify_task_status == HOSTING_TASK_ERROR){
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

  $branch_options = array_combine((array) $project_node->git_branches, (array) $project_node->git_branches);

  $form['default_platforms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Project Environments'),
    '#description' => t('Choose the environments you would like to create for this project.  You will be able to create more or change the branch later.'),
    '#theme' => 'devshop_project_create_platforms_table',
    '#tree' => TRUE,
  );
  foreach ($default_platforms as $env) {
    $form['default_platforms'][$env] = array(
      '#tree' => TRUE,
      '#prefix' => '<div class="platform">',
      '#suffix' => '</div>',
    );
    $form['default_platforms'][$env]['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => $env,
      '#default_value' => !empty($project->default_platforms[$env]),
    );
    $form['default_platforms'][$env]['branch'] = array(
        '#type' => 'select',
        '#default_value' => isset($project->default_platforms[$env])? $project->default_platforms[$env]: NULL,
        '#options' => array(t('Choose a branch...')) + $branch_options,
      );
  }

  $form['branch_platforms'] = array(
    '#type' => 'checkboxes',
    '#multiple' => 'true',
    '#title' => t('Branch Environments'),
    '#description' => t('Create new environments based on the project\'s git branches.'),
    '#default_value' => $project->branch_platforms,
    '#options' => $branch_options
  );
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

  // Get last platforms, in case this form is re-submitted
  $old_platforms = array_merge($project->default_platforms, $project->branch_platforms);

  // Get platforms from form values and save into our object
  $project->default_platforms = array_filter($values['default_platforms']);
  $project->branch_platforms = array_filter($values['branch_platforms']);

  // Create these platforms
  $all_platforms = array_merge($project->default_platforms, $project->branch_platforms);
  foreach ($all_platforms as $platform_name => $branch) {
    // If platform hasn't been created yet, do so now!
    if (!empty($branch) && empty($project->platforms[$platform_name])){
      // Save the damn platform nodes
      $platform = new stdClass;

      // hosting_platform fields
      $platform->title = $project->title . '_' . $platform_name;
      $platform->publish_path = $project->code_path . '/' . $platform_name;

      $servers = hosting_get_servers('http');
      $platform->web_server = variable_get('devshop_projects_default_web_server', key($servers));

      $platform->git_branch = $branch;
      $platform->project = $project->title;
      $platform->environment = $platform_name;

      $platform_node = _devshop_projects_node_create('platform', $platform);
      $project->platforms[$platform_name] = $platform_node->nid;
    }
    // If the platform already exists, change the platform node branch and save it.
    // @TODO: only re-save the node if the chosen branch changed.
    else {
      $platform_node = node_load($project->platforms[$platform_name]);
      $platform_node->git_branch = $branch;
      node_save($platform_node);
    }
  }

  // For all removed platforms, trigger a delete task
  $removed_platforms  = array_diff_key($old_platforms, $all_platforms);
  foreach ($removed_platforms as $platform_name => $branch) {

    // @TODO: Determine what to do here based on task status...
    // if verify task hasn't even run yet (and has never run) we can just delete
    // the platform node.

    // Create 'delete' task for this project
    hosting_add_task($project->platforms[$platform_name], 'delete');

    // Remove from project object
    unset($project->platforms[$platform_name]);
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
    if (!isset($project->platforms[$name])){
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
    $note .= '<p class="wait">' . t('<em>NOTE: It is a <a href="http://drupal.org/node/1870416" target>known bug</a> that this page doesn\'t automatically reload.  Please reload the page manually once the task is complete.</em>') . '</p>';

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