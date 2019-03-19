<?php
include 'step-1.inc';
include 'step-2.inc';
include 'step-3.inc';
include 'step-4.inc';


/**
 * Project Create Wizard Page
 */
function devshop_projects_create_wizard($step = NULL) {
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
  $project = ctools_object_cache_get('project', 'devshop_project');

  // Setup Step.
  if ($step == NULL) {
    drupal_goto('projects/add/' . current(array_keys($form_info['order'])));
  }

  // Create default project object
  if (empty($project) || !isset($project->git_url)) {
    // set form to first step -- we have no data
    $step = current(array_keys($form_info['order']));
    $project = new stdClass();
    $project->step = $step;
    $project->git_url = '';
    $project->nid = NULL;
    $project->name = '';

    // These are empty until we have our project name
    $project->code_path = '';
    $project->base_url = '';

    // ** set the storage object so its ready for whatever comes next
    ctools_object_cache_set('project', 'devshop_project', $project);
  }
  else {
    // Load latest project object from database and use that for the ctools cache.
    $project_node = node_load($project->nid);
    $project = $project_node->project;

    // If we have a name but no code_path or base URL.
    if (!empty($project->name) && empty($project->code_path)) {
      $project->code_path = variable_get('devshop_project_base_path', '/var/aegir/projects') . '/' . $project->name;
    }
    if (!empty($project->name) && empty($project->base_url)) {
      $project->base_url = variable_get('devshop_project_environment_url_pattern', "@project.@environment.@hostname");
    }

    // Save the current step
    $project->step = $step;
    ctools_object_cache_set('project', 'devshop_project', $project);
  }

  // Check verification status
  if (!empty($project->nid)) {
    $tasks = hosting_task_fetch_tasks($project->nid);
  }
  if (isset($tasks['verify']['nid'])) {
    // Get "verify" task status for the project
    $project->verify_task_status = isset($tasks['verify']['task_status']) ? $tasks['verify']['task_status'] : HOSTING_TASK_ERROR;
    $project->verify_task_nid = $tasks['verify']['nid'];
  }
  else {
    $project->verify_task_status = NULL;
    $project->verify_task_nid = NULL;
  }

  // If project verification failed, we need to ask for a new git url.
  if ($project->verify_task_status == HOSTING_TASK_ERROR && !empty($project_node->nid)) {
    $messages = db_query("SELECT * FROM {hosting_task_log} WHERE nid = :nid AND type LIKE :type ORDER BY vid, lid", array(':nid' => $project->verify_task_nid, ':type' => 'p_%'));

    foreach ($messages as $message) {
      if ($message->type == 'p_info') {
        $messages_to_user[] = $message->message;
      }
    }
    $project->verify_error = theme('devshop_ascii', array('output' => implode('\n', $messages_to_user)));


    // If not on the first step, go to it.
    if ($step != current(array_keys($form_info['order']))) {
      drupal_goto('projects/add/' . current(array_keys($form_info['order'])));
    }
  }
  else {
    $project->verify_error = NULL;
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
function devshop_projects_create_wizard_info() {
  return array(
    'id' => 'devshop_project_create',
    'path' => "projects/add/%step",
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
  $cache = ctools_object_cache_set('project', 'devshop_project', $project);
}




/**
 * CANCEL callback
 * Callback generated when the 'cancel' button is clicked.
 * Remove the project data cache and send back to projects page.
 */
function devshop_projects_create_wizard_cancel(&$form_state) {
  // Update the cache with changes.
  $project = &$form_state['project'];
  ctools_object_cache_clear('project', 'devshop_project');

  // Redirect to projects list
  $form_state['redirect'] = 'projects';

  // If we have a project node, create a "delete" hosting task
  if (!empty($project->nid)) {
    hosting_add_task($project->nid, 'delete');
  }

  // Message
  drupal_set_message(t('Project creation cancelled.'));

  // Removing last step session variable.
  unset($_SESSION['last_step']);

}

/**
 * FINISH callback
 * Callback generated when the add page process is finished.
 */
function devshop_projects_create_wizard_finish(&$form_state) {

  $project = &$form_state['project'];
  $project_node = node_load($project->nid);

  // Save the extra options to the project node.
  $project_node->project->install_profile = $form_state['values']['install_profile'];

  // Create the site nodes, saving to the environment.
  // @TODO: Can we speed things up here by only running install for the first,
  // then  "Cloning" to create the rest?
  foreach ($project_node->project->environments as $environment_name => &$environment) {
    // @TODO: Does this set the http_server as well?? Doesn't look like it.
    $db_server = $environment->db_server_nid;
    $site_node = devshop_projects_create_site($project_node->project, node_load($environment->platform), $environment_name, $db_server);
    $environment->site = $site_node->nid;
  }

  // Set to not verify and to publish.
  $project_node->no_verify = TRUE;
  $project_node->status = 1;
  node_save($project_node);

  ctools_object_cache_clear('project', 'devshop_project');
  $form_state['redirect'] = 'node/' . $project_node->nid;

  // Removing last step session variable.
  unset($_SESSION['last_step']);

  // Friendly message
  drupal_get_messages('status');
  drupal_set_message(t('Your project has been created.  Your sites are being installed.'));
}

/**
 * Returns JSON showing the state of the project
 */
function devshop_projects_add_status($type = 'platform') {
  $return = array();

  // Get Project Cache
  ctools_include('wizard');
  ctools_include('object-cache');
  $project = ctools_object_cache_get('project', 'devshop_project');


  $project_node = node_load($project->nid);

  $all_tasks_completed = TRUE;
  $nids = array();

  // When checking project...
  if ($type == 'project') {
    $nids = array($project_node->nid);
  }

  // When checking platforms...
  if ($type == 'platform') {
    foreach ($project_node->project->environments as $name => $environment) {
      $nids[] = $environment->platform;
    }
  }

  // Check verification task for all nids
  foreach ($nids as $nid) {
    $platform = node_load($nid);
    $profiles_shortnames = hosting_get_profiles($platform->nid, 'short_name');
    if (is_array($profiles_shortnames) && !empty($profiles_shortnames)) {
      $install_profiles = array_combine($profiles_shortnames, (array) hosting_get_profiles($platform->nid));
    }
    else {
      $install_profiles = array();
    }

    $status = _hosting_parse_error_code($platform->environment->last_task_node->task_status);
    if ($status == 'Queued') {
      $status = '...';
    }
    $return['tasks'][$nid] = array(
      'status' => $status,
      'version' => !empty($platform->release->version)? $platform->release->version: '',
      'profiles' => implode(', ', $install_profiles),
    );
    // If task is not completed, mark all tasks not complete.
    if ($platform->environment->last_task_node->task_status == HOSTING_TASK_SUCCESS || $platform->environment->last_task_node->task_status == HOSTING_TASK_ERROR || $platform->environment->last_task_node->task_status == HOSTING_TASK_WARNING) {
      continue;
    }
    else {
      $all_tasks_completed = FALSE;
    }

  }
  $return['tasks_complete'] = $all_tasks_completed;
  drupal_json_output($return);
  exit;
}

/**
 * Helper for adding reloading feature to form
 */
function devshop_form_reloader(&$form, $type = 'platform') {
  // Add JS that reloads the page when tasks finish.
  $form['item'] = array(
    '#type' => 'item',
    '#value' => '',
    '#weight' => 10
  );
  $settings['devshopReload'] = array(
    'type' => $type,
    'delay' => 1000,
  );

  drupal_add_js($settings, array('type' => 'setting'));
  drupal_add_js(drupal_get_path('module', 'devshop_projects') . '/inc/reload.js');
}
