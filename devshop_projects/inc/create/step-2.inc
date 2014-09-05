<?php

/*********
* STEP 2
* Project Settings
*********/

/**
* STEP 2: Form
*/
function devshop_project_create_step_settings(&$form, &$form_state) {
$project = &$form_state['project'];

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

}

/**
* STEP 2: Submit
*/
function devshop_project_create_step_settings_submit(&$from, &$form_state) {
  $project = &$form_state['project'];
  $project->drupal_path = $form_state['values']['drupal_path'];
}
