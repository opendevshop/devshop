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
    $form['project']['environments'][$name]['name'] = array(
      '#type' => 'textfield',
      '#default_value' => $env_title,
      '#size' => 6,
      '#maxlength' => 64,
      '#attributes' => array(
        'placeholder' => t('name'),
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
      '#type' => 'select',
      '#options' => devshop_projects_git_ref_options($project),
      '#default_value' => $environment->git_ref,
    );

    // HTTP Server select.
    $http_servers = hosting_get_servers('http');
    if (count($http_servers) == 1) {
      $form['project']['environments'][$name]['settings']['web_server'] = array(
          '#type' => 'value',
          '#value' => $project->settings->default_environment['web_server'],
      );
    }
    elseif (count($http_servers) > 1) {
      $form['project']['environments'][$name]['settings']['web_server'] = array(
        '#title' => t('Web server'),
        '#type' => 'select',
        '#options' => $http_servers,
        '#default_value' => $environment->settings->web_server?
          $environment->settings->web_server:
          $project->settings->default_environment['web_server']
      );
    }

    // DB Server select.
    $db_servers = hosting_get_servers('db');
    if (count($db_servers) == 1) {
      $form['project']['environments'][$name]['settings']['db_server'] = array(
          '#type' => 'value',
          '#value' => $project->settings->default_environment['db_server'],
      );
    }
    elseif (count($db_servers) > 1) {
      $form['project']['environments'][$name]['settings']['db_server'] = array(
        '#title' => t('Database server'),
        '#type' => 'select',
        '#options' => $db_servers,
        '#default_value' => $environment->settings->db_server?
          $environment->settings->db_server:
          $project->settings->default_environment['db_server']
      );
    }

    // Add Environment
    $form['add_environment'] = array(
      '#type' => 'submit',
      '#value' => t('Add environment'),
      '#name' => 'add_environment',
      '#submit' => array('devshop_projects_save_environments'),
      '#prefix' => '<div class="project-add-environment-button">',
      '#suffix' => '</div>',
    );
  }
  $form['project']['environments']['NEW']['name']['#attributes']['autofocus'] = 'autofocus';
}


/**
 * STEP 3: Validate
 */
function devshop_project_create_step_environments_validate(&$form, &$form_state) {
  $project = &$form_state['project'];

  $values = &$form_state['values'];

  // Check environment titles
  foreach ($values['project']['environments'] as $env => $env_settings) {
    // Check for illegal chars
    if ($env != 'NEW' && !empty($env_settings['name'])) {
      if (!preg_match('!^[a-z0-9_]+$!', $env_settings['name'])) {
        $form_item = 'environments][' . $env . '][name';
        form_set_error($form_item, t('The environment name must contain only lowercase letters, numbers, and underscores.'));
      }
    }
  }

  // Reject if empty
  if (count($values['project']['environments']) < 1) {
    if ($form_state['clicked_button']['#name'] == 'add_environment') {
      form_set_error('project][environments][NEW][name', t('Name this environment before you add another.'));
    } else {
      form_set_error('project][environments][NEW][name', t('You must add at least one environment.'));
    }
  }
}

/**
 * Functionality for add a new environment.
 */
function devshop_projects_save_environments($form, &$form_state) {

  $environments = $form_state['values']['project']['environments'];

  if (!empty($environments['NEW']['name'])) {
    $new_environment_name = $environments['NEW']['name'];
    $environments[$new_environment_name] = $environments['NEW'];
  }
  unset($environments['NEW']);

  // Delete all environments
  db_query('DELETE FROM {hosting_devshop_project_environment} WHERE project_nid = %d', $form_state['values']['nid']);

  // Create all environments
  foreach ($environments as $environment) {
    $name = trim($environment['name']);
    if (!empty($name)) {
      db_query('INSERT INTO {hosting_devshop_project_environment} (project_nid, name, git_ref, platform, settings) VALUES (%d, "%s", "%s", %d, "%s");', $form_state['values']['nid'], $environment['name'], $environment['git_ref'], $environment['platform'], serialize($environment['settings']));
    }
  }

  // Go back to the same page.
  if ($form_state['clicked_button']['#name'] == 'add_environment') {
    drupal_goto('hosting/projects/add/environments');
  }
}

/**
 * STEP 3: SUBMIT
 */
function devshop_project_create_step_environments_submit(&$form, &$form_state) {

  // Save all environments.
  devshop_projects_save_environments($form, $form_state);

  // Get project and reset properties..
  $project_node = node_load($form_state['values']['nid'], NULL, TRUE);
  $project = $project_node->project;

  // Create these platforms, if they don't exist yet
  foreach ($project->environments as $name => $environment) {

    // If platform exists, it's because user went back in the wizard.
    $platform_nid = $environment->platform;

    // If the platform already exists, update settings and verify it again.
    if ($platform_nid) {
      // @TODO: Apply settings and re-save platform.
      //hosting_add_task($platform_nid, 'verify');

      $platform = node_load($platform_nid);
      $platform->publish_path = $project->code_path . '/' . $environment->name . '/' . $project->drupal_path;
      $environment->platform = $platform_nid;
      $platform->web_server = $environment->settings->web_server;
      $platform->db_server = $environment->settings->db_server;
      node_save($platform);
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
      $platform->web_server = $environment->settings->web_server;
      $platform->db_server = $environment->settings->db_server;
      $platform_node = _devshop_projects_node_create('platform', $platform);
      $environment->platform = $platform_node->nid;
    }

    // Update environment with our platform
    db_query('UPDATE {hosting_devshop_project_environment} SET platform = %d WHERE project_nid = %d AND name = "%s"', $environment->platform, $form_state['values']['nid'], $environment->name);
  }

  // For all removed platforms, trigger a delete task
  // @TODO: Get this working again.
//  $removed_environments = array_diff_key($project->environments, $form_state['values']['project']['environments']);
//
//  foreach ($removed_environments as $environment_name => $settings) {
//    // @TODO: Determine what to do here based on task status...
//    // if verify task hasn't even run yet (and has never run) we can just delete
//    // the platform node.
//
//    // Create 'delete' task for the removed platform
//    $platform_nid = $settings['platform'];
//    if ($platform_nid) {
//      hosting_add_task($platform_nid, 'delete');
//    }
//  }
}