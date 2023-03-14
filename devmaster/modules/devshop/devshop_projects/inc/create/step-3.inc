<?php

/**********
 * STEP 3
 * Project Environments
 *********/

/**
 * STEP 3: Form
 */
function devshop_project_create_step_environments($form, &$form_state) {
  global $user;
  $project = &$form_state['project'];
  if ($project->verify_task_status == HOSTING_TASK_QUEUED || $project->verify_task_status == HOSTING_TASK_PROCESSING) {
    $note = '<p class="wait">' . t('Please wait while we connect to your repository and determine any branches.') . '</p>';
    $form['note'] = array(
      '#markup' => $note,
    );
    $form['#no_next'] = TRUE;

    // Add code to reload the page when complete.
    devshop_form_reloader($form, 'project');
    return $form;
  }

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project->nid,
  );
  $form['message'] = array(
    '#prefix' => "<div class='alert alert-info'>",
    '#suffix' => '</div>',
    '#markup' => t('Create as many new environments as you would like. You can create and destroy environments at any time.'),
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

  // If there was a problem with loading branches or tags, hide the form and show a link to re-verify.
  if (empty($project->settings->git['branches']) && empty($project->settings->git['tags'])) {
    $form['error'] = array(
      '#markup' => t('Unable to devshop_projects_create_settings_formfind any branches or tags. Please !verify.', array(
          '!verify' => l(t('Re-verify the project'), "hosting_confirm/{$project->nid}/project_verify", array(
            'query' => array('token' => drupal_get_token($user->uid)),
            'attributes' => array(
              'class' => array('btn btn-success'),
            ),
          ))
        )),
      '#prefix' => '<div class="jumbotron alert alert-danger">',
      '#suffix' => '</div>',
    );
    return $form;
  }

  // Ensure a blank row exists (happens when using 'Back' button).
  if (!isset($project->environments['NEW']) || !is_array($project->environments['NEW'])) {
    $project->environments['NEW'] = array();
  }
  foreach ($project->environments as $name => $environment) {
    if (empty($name)) {
      continue;
    }
    $environment = convert_to_object($environment);
    $form['project']['environments'][$name] = devshop_project_environment_create_form_element($project, $environment);

    // Add Environment
    $form['add_environment'] = array(
      '#type' => 'submit',
      '#value' => t('Add environment'),
      '#name' => 'add_environment',
      '#prefix' => '<div class="project-add-environment-button">',
      '#suffix' => '</div>',
      '#states' => array(
        'visible' => array(
          ':input[name="project[environments][NEW][name]"]' => array(
            'filled' => true,
          )
        )
      )
    );
  }
  $form['project']['environments']['NEW']['name']['#attributes']['autofocus'] = 'autofocus';

  $form['install_method'] = devshop_environment_install_method_form($project);
  $form['install_method']['method']['#title'] = t('Default Install Method');
  $form['install_method']['#description'] = t('Environments will be installed with this method by default. You can choose alternative methods when creating environments manually.');
  
  return $form;
}


/**
 * STEP 3: Validate
 */
function devshop_project_create_step_environments_validate(&$form, &$form_state) {
  $project = &$form_state['project'];

  $values = &$form_state['values'];

  if (isset($values['project'])) {

    // Check environment titles
    foreach ($values['project']['environments'] as $env => $env_settings) {
      // Check for illegal chars
      if ($env != 'NEW' && !empty($env_settings['name'])) {
        if (!preg_match('!^[a-z0-9_]+$!', $env_settings['name'])) {
          $form_item = 'project][environments][' . $env . '][name';
          form_set_error($form_item, t('The environment name must contain only lowercase letters, numbers, and underscores.'));
        }
      }

      // Ensure NEW env name does not already exist in the environments array.
      if ($env == 'NEW' && array_key_exists($env_settings['name'], $values['project']['environments'])) {
        $form_item = 'project][environments][' . $env . '][name';
        form_set_error($form_item, t('The environment name must be unique. %name is already in use.', array(
          '%name' => $env_settings['name'],
        )));
      }
    }

    // Reject if empty
    if (count($form_state['values']['project']['environments']) < 1) {
      if ($form_state['clicked_button']['#name'] == 'add_environment') {
        form_set_error('project][environments][NEW][name', t('Name this environment before you add another.'));
      }
      else {
        form_set_error('project][environments][NEW][name', t('You must add at least one environment.'));
      }
    }
  }
}

///**
// * Functionality for add a new environment.
// */
//function devshop_projects_save_environments($form, &$form_state) {
//
//  $environments = $form_state['values']['project']['environments'];
//
//  if (!empty($environments['NEW']['name'])) {
//    $new_environment_name = $environments['NEW']['name'];
//    $environments[$new_environment_name] = $environments['NEW'];
//  }
//  unset($environments['NEW']);
//
//  // Delete all environments
//  db_delete('hosting_devshop_project_environment')
//    ->condition('project_nid', $form_state['values']['nid'])
//    ->execute();
//
//  // Create all environments
//  foreach ($environments as $environment) {
//    $name = trim($environment['name']);
//    if (!empty($name)) {
//      $id = db_insert('hosting_devshop_project_environment')
//              ->fields(array(
//                'project_nid' => $form_state['values']['nid'],
//                'name' => $environment['name'],
//                'git_ref' => $environment['git_ref'],
//                'platform' => $environment['platform'],
//                'settings' => serialize($environment['settings']),
//              ))
//              ->execute();
//    }
//  }
//
//  // Go back to the same page.
//  if ($form_state['clicked_button']['#name'] == 'add_environment') {
//    drupal_goto('projects/add/environments');
//  }
//}

/**
 * STEP 3: SUBMIT
 */
function devshop_project_create_step_environments_submit(&$form, &$form_state) {
  // Get project and reset properties..
  $project_node = node_load($form_state['values']['nid'], NULL, TRUE);
  $project = $project_node->project;
  if (isset($form_state['project']->code_path) && empty($project->code_path)) {
    $project->code_path = $form_state['project']->code_path;
  }
  if (isset($form_state['project']->drupal_path) && empty($project->drupal_path)) {
    $project->drupal_path = $form_state['project']->drupal_path;
  }

  // Save environments data
  $environments = $form_state['values']['project']['environments'];

  // Create all environments
  if ($form_state['clicked_button']['#name'] == 'add_environment') {

    if (isset($environments['NEW']) && !empty($environments['NEW']['name'])) {
      $new_name = $environments['NEW']['name'];
      $environments[$new_name] = $environments['NEW'];
      unset($environments['NEW']);
    }

    $project->environments = $environments;
    ctools_object_cache_set('project', 'devshop_project', $project);
    $form_state['redirect'] = 'projects/add/environments';
  }
  else {
    foreach ($environments as $name => $environment) {
      $environment = (object) $environment;
      $environment->settings = (object) $environment->settings;
      $name = trim($environment->name);
      if (!empty($name)) {
        hosting_create_environment($project, $name, $environment->git_ref, NULL, $environment->settings, 'create', $environment->git_remote, TRUE);
      }
    }
  }
}

/**
 *
 */
function devshop_project_environment_create_form_element($project, $environment) {

  $name = isset($environment->name) ? $environment->name : '';

  $element = array(
    '#tree' => TRUE,
    '#theme' => 'devshop_projects_settings_table',
  );

  // Environment properties
  // Generate field prefix and suffix using domain name pattern.
  if (variable_get('devshop_projects_allow_custom_base_url')) {
    $pattern = $project->base_url;
  }
  else {
    $pattern = variable_get('devshop_project_environment_url_pattern', '@project.@environment.@hostname');
  }

  $labels = explode('@environment', strtr($pattern, array(
    '@project' => $project->name,
    '@hostname' => $_SERVER['SERVER_NAME'],
  )));
  $element['name'] = array(
    '#type' => 'textfield',
    '#default_value' => $name,
    '#size' => 6,
    '#maxlength' => 64,
    '#attributes' => array(
      'placeholder' => t('name'),
    ),
    '#field_prefix' => '<div class="input-group"><div class="input-group-addon">http://' . $labels[0] . '</div>',
    '#field_suffix' => '<div class="input-group-addon input-group-addon-url">' . $labels[1] .'</div>
        </div>',
  );
  $element['site'] = array(
    '#type' => 'value',
    '#value' => isset($environment->site) ? $environment->site : NULL,
  );
  $element['platform'] = array(
    '#type' => 'value',
    '#value' => isset($environment->platform) ? $environment->platform : NULL,
  );
  $element['git_ref'] = array(
    '#type' => 'select',
    '#bootstrap_ignore_pre_render' => TRUE,
    '#options' => devshop_projects_git_ref_options($project),
    '#default_value' => isset($environment->git_ref) ? $environment->git_ref : 'HEAD',
  );

  // HTTP Server select.
  $http_servers = hosting_get_servers('http', FALSE);

  // Determine default web server.
  $default_web_server = isset($environment->settings, $environment->settings->web_server) ?
    $environment->settings->web_server :
    $project->settings->default_environment['web_server'];

  // Fallback if environment or default not found.
  if (empty($default_web_server)) {
    $default_web_server = key($http_servers);
  }

  if (count($http_servers) == 1) {
    $element['settings']['web_server'] = array(
      '#type' => 'value',
      '#value' => $default_web_server,
    );
  }
  elseif (count($http_servers) > 1) {
    $element['settings']['web_server'] = array(
      '#title' => t('Web server'),
      '#type' => 'select',
      '#options' => $http_servers,
      '#default_value' => $default_web_server
    );
  }

  // DB Server select.
  $db_servers = hosting_get_servers('db', FALSE);

  // Determine default db server.
  $default_db_server = isset($environment->settings, $environment->settings->db_server) ?
    $environment->settings->db_server :
    $project->settings->default_environment['db_server'];

  // Fallback if environment or default not found.
  if (empty($default_db_server) || !isset($db_servers[$default_db_server])) {
    $default_db_server = key($db_servers);
  }

  if (count($db_servers) == 1) {
    $element['settings']['db_server'] = array(
      '#type' => 'value',
      '#default_value' => $default_db_server,
    );
  }
  elseif (count($db_servers) > 1) {
    $element['settings']['db_server'] = array(
      '#title' => t('Database server'),
      '#type' => 'select',
      '#options' => $db_servers,
      '#default_value' => $default_db_server
    );
  }

  return $element;
}
