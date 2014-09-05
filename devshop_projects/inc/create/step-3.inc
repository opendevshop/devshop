<?php

/**********
 * STEP 3
 * Project Environments
 *********/

/**
 * STEP 3: Form
 */
function devshop_project_create_step_environments(&$form, &$form_state) {
  $project_cache = & $form_state['project'];
  $project_node = node_load($project_cache->project_nid);
  $project = $project_node->project;

  if ($project_cache->verify_task_status == HOSTING_TASK_QUEUED || $project_cache->verify_task_status == HOSTING_TASK_PROCESSING) {
    $note = '<p class="wait">' . t('Please wait while we connect to your repository and determine any branches.') . '</p>';
    $project_cache->no_next = TRUE;

    $form['note'] = array(
      '#type' => 'markup',
      '#value' => $note,
    );
    $form['not_ready'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );

    // Add code to reload the page when complete.
    devshop_form_reloader($form, 'project');
    return;
  }

  // this JS handles the form element hiding/showing
  $path = drupal_get_path('module', 'devshop_projects') . '/inc/environments.js';
  drupal_add_js($path);

  $settings = module_invoke_all('devshop_project_settings', $project_node);
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,
  );
  $form['project'] = array(
    '#tree' => TRUE,
  );
  $form['project']['environments'] = array(
    '#theme' => 'devshop_projects_create_settings_form',
    '#tree' => TRUE,
    '#prefix' => '<div id="environments-wrapper">',
    '#suffix' => '</div>',
  );

  // Ensure a blank row exists (happens when using 'Back' button)
  if (!is_array($project->environments['NEW'])) {
    $project->environments['NEW'] = array();
  }
  foreach ($project->environments as $name => $environment) {
    // No platforms exist yet
    if ($name == 'NEW') {
      $env_title = '';
    } else {
      $env_title = $name;
    }

    $form['project']['environments'][$name] = array(
      '#tree' => TRUE,
      '#type' => 'fieldset',
      '#theme' => 'devshop_projects_settings_table',
    );

    // Environment properties
    $form['project']['environments'][$name]['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Environment Name'),
      '#default_value' => $env_title,
      '#size' => 6,
      '#maxlength' => 64,
      '#attributes' => array(
        'placeholder' => t('name'),
        'autofocus' => 'autofocus',
      ),
      '#field_prefix' => 'http://',
      '#field_suffix' => "." . $project->base_url,
    );
    $form['project']['environments'][$name]['site'] = array(
      '#type' => 'value',
      '#value' => $environment->site,
    );
    $form['project']['environments'][$name]['platform'] = array(
      '#type' => 'value',
      '#value' => $environment->platform,
    );
    $form['project']['environments'][$name]['git_ref'] = array(
      '#title' => t('Git Branch/Tag'),
      '#type' => 'select',
      '#options' => devshop_projects_git_ref_options($project),
      '#default_value' => $environment->git_ref,
    );

    // Environment settings
    // Add environment settings form elements
    foreach ($settings as $setting_id => $setting) {
      $form['project']['environments'][$name]['settings'][$setting_id] = $setting;
      $form['project']['environments'][$name]['settings'][$setting_id]['#default_value'] = $environment->settings->{$setting_id};
      $form['project']['environments'][$name]['settings'][$setting_id]['#attributes']['title'] = $setting['#description'];
      $form['project']['environments'][$name]['settings'][$setting_id]['#description'] = '';
    }

//Now add button.
    $form['add_environment'] = array(
      '#type' => 'submit',
      '#value' => t('Add environment'),
      '#name' => 'add_environment',
      '#submit' => array('devshop_projects_create_wizard_add_new_environment'),
      '#prefix' => '<div class="project-add-environment-button">',
      '#suffix' => '</div>',
    );
  }
}


/**
 * STEP 3: Validate
 */
function devshop_project_create_step_environments_validate(&$form, &$form_state) {
  $project = &$form_state['project'];

  $values = &$form_state['values'];

// Changes NEW environment data to $title
  if ($values['project']['environments']['NEW']['title']) {
    $new_env = $values['project']['environments']['NEW']['title'];
    $new_env_settings = $values['project']['environments']['NEW'];
    $values['project']['environments'][$new_env] = $new_env_settings;

// Create the next NEW environment.  Unset makes sure its always last.
    unset($values['project']['environments']['NEW']);

// If "add environment" button clicked, add another row.
    if ($form_state['clicked_button']['#name'] == 'add_environment') {
      $values['project']['environments']['NEW'] = array();
    }
  } else {
    unset($values['project']['environments']['NEW']);
  }

// Check environment titles
  foreach ($values['project']['environments'] as $env => $env_settings) {
// Check for illegal chars
    if ($env != 'NEW' && !empty($env_settings['title'])) {
      if (!preg_match('!^[a-z0-9_]+$!', $env_settings['title'])) {
        $form_item = 'environments][' . $env . '][title';
        form_set_error($form_item, t('The environment name must contain only lowercase letters, numbers, and underscores.'));
      }
    }
  }

// Reject if empty
  if (count($values['project']['environments']) < 1) {
    if ($form_state['clicked_button']['#name'] == 'add_environment') {
      form_set_error('project][environments][NEW][title', t('Name this environment before you add another.'));
    } else {
      form_set_error('project][environments][NEW][title', t('You must add at least one environment.'));
    }
  }
}

/**
 * STEP 3: SUBMIT
 */
function devshop_project_create_step_environments_submit(&$form, &$form_state) {
  // Get project and reset properties..
  $project = $form_state['project'];
  $project_node = node_load($project->project_nid);

  $settings = module_invoke_all('devshop_project_settings', $project_node);

  // Create these platforms, if they don't exist yet
  foreach ($form_state['values']['project']['environments'] as $name => $environment) {

    $environment = (object) $environment;

    // If platform exists, it's because user went back in the wizard.
    $platform_nid = $project->environments[$name]->platform;

    // If the platform already exists, update settings and verify it again.
    if ($platform_nid) {
      // @TODO: Apply settings and re-save platform.
      hosting_add_task($platform_nid, 'verify');
    }

    // If platform hasn't been created yet, do so now!
    else {

      // Build platform node
      $platform = new stdClass;
      $platform->title = $project->name . '_' . $name;

      // Platform publish_path
      if (!empty($project->drupal_path)) {
        $platform->publish_path = $project->code_path . '/' . $environment->name . '/' . $project->drupal_path;
      } else {
        $platform->publish_path = $project->code_path . '/' . $environment->name;
      }

      // Other attributes
      $platform->web_server = $environment->settings['web_server'];
      foreach ($settings as $setting_name => $element) {
        if ($element['#node_type'] == 'platform') {
          $platform->{$setting_name} = $environment->settings[$setting_name];
        }
      }
      $platform_node = _devshop_projects_node_create('platform', $platform);
    }
    $environment->platform = $platform_node->nid;
    $project->environments[$name] = (object) $environment;
  }

  // Somehow, NEW environment still appears!
  unset($project_node->project->environments['NEW']);

  // For all removed platforms, trigger a delete task
  $removed_environments = array_diff_key($project_node->project->environments, $form_state['values']['project']['environments']);

  foreach ($removed_environments as $environment_name => $settings) {
    // @TODO: Determine what to do here based on task status...
    // if verify task hasn't even run yet (and has never run) we can just delete
    // the platform node.

    // Create 'delete' task for the removed platform
    $platform_nid = $settings['platform'];
    if ($platform_nid) {
      hosting_add_task($platform_nid, 'delete');
    }
  }

  // Save the project node but don't verify.  This triggers environment saving.
  $project_node->project = $project;
  $project_node->no_verify = TRUE;
  node_save($project_node);
}