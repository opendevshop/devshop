<?php

/*********
 * STEP 2
 * Project Settings
 *********/

/**
 * STEP 2: Form
 */
function devshop_project_create_step_settings($form, &$form_state) {

  // Load project node and it's form
  $project = &$form_state['project'];
  $node = node_load($project->nid);

  $form_state['build_info']['args'] = array($node);
  $project_form = devshop_projects_form($node);

  // Remove node buttons
  unset($project_form['actions']);

  // Merge project node form into ctools wizard form.
  $form = array_merge($project_form, $form);

  // Make sure the project isn't re-verified on this page.
  $form['no_verify'] = array(
    '#type' => 'value',
    '#value' => TRUE,
  );

  // Add additional additional submit and validate handlers for the node form.
//  $form['buttons']['next']['#submit'][] = 'node_form_submit';
  $form['buttons']['next']['#submit'][] = 'ctools_wizard_submit';
//  $form['buttons']['next']['#validate'][] = 'node_form_validate';

  drupal_add_js(drupal_get_path('module', 'devshop_projects') . '/inc/create/create.js');

  // Remove "live environment" selector.
  $form['project']['settings']['live']['live_environment']['#type'] = 'value';

  return $form;
}

/**
 * STEP 2: Validate
 */
function devshop_project_create_step_settings_validate(&$from, &$form_state) {
  // Nothing is needed here.  We've replaced our validator with node_form_validate,
  // which passes through to devshop_projects_validate()
}

/**
 * STEP 2: Submit
 */
function devshop_project_create_step_settings_submit(&$from, &$form_state) {
  // Nothing is needed here.  We've replaced our validator with node_form_validate,

}
