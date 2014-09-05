<?php

/*********
* STEP 2
* Project Settings
*********/

/**
* STEP 2: Form
*/
function devshop_project_create_step_settings(&$form, &$form_state) {

  // Load project and it's form
  $project = &$form_state['project'];
  $node = node_load($project->project_nid);
  $form += devshop_projects_form($node);
  $form['#node'] = $node;
  devshop_pull_form_alter($form, $form_state, 'project_node_form');

  // Remove "environment" selector.
  $form['project']['settings']['live']['live_environment']['#type'] = 'value';

  // Add "Path to Drupal".
  $form['drupal_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Drupal'),
    '#description' => t('Enter the relative path to the index.php file in your repository. Leave blank if index.php is in the root.'),
    '#size' => 40,
    '#default_Value' => $project->drupal_path,
    '#maxlength' => 255,
  );
}

/**
* STEP 2: Validate
*/
function devshop_project_create_step_settings_validate(&$from, &$form_state) {
  // Pass through to devshop validate
  //  devshop_projects_validate($node, $form);

  dsm($form_state);
  form_set_error('wtf', 'wtf');
}

/**
* STEP 2: Submit
*/
function devshop_project_create_step_settings_submit(&$from, &$form_state) {
  $project = &$form_state['project'];
  $project->drupal_path = $form_state['values']['drupal_path'];


  $project->settings = $form_state['values']['project']['settings'];

  // Save the settings to the project node
}
