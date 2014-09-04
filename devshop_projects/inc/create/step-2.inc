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

// Save code path and base url.
$project_node = node_load($project->project_nid);
$project_node->project->code_path = $form_state['values']['code_path'];
$project_node->project->drupal_path = $form_state['values']['drupal_path'];
$project_node->project->base_url = $form_state['values']['base_url'];

$project_node->no_verify;
node_save($project_node);
}
