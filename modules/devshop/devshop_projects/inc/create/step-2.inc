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

  $node_type = 'project';
  $form_id = $node_type . '_node_form';
  $form_state['build_info']['args'] = array($node);
  $form_state['build_info']['files'][] = array('inc', 'node', 'node.pages');
  module_load_include('inc', 'node', 'node.pages');
  $project_form = drupal_retrieve_form($form_id, $form_state);
  drupal_prepare_form($form_id, $project_form, $form_state);
  // Remove node buttons
  $actions = $project_form['actions'];
  unset($project_form['actions']);

  // Merge project node form into ctools wizard form.
  $form = array_merge($project_form, $form);

  // Make sure the project isn't re-verified on this page.
  $form['no_verify'] = array(
    '#type' => 'value',
    '#value' => TRUE,
  );

  // Add additional additional submit and validate handlers for the node form.
  $form['buttons']['next']['#submit'] = $actions['submit']['#submit'];
  $form['buttons']['next']['#submit'][] = 'ctools_wizard_submit';

  drupal_add_js(drupal_get_path('module', 'devshop_projects') . '/inc/create/create.js');

  // Remove "live environment" selector.
  $form['project']['codebase']['live_environment']['#type'] = 'value';
  $form['project']['codebase']['live_environment']['#suffix'] = '';

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
 *
 * If webroot was updated, update platforms path.
 */
function devshop_project_create_step_settings_submit(&$form, &$form_state) {

  // Update any platforms if the path to drupal changed
  if ($form['project']['codebase']['drupal_path']['#default_value'] != $form_state['values']['project']['drupal_path']) {
    $environments = db_query("
      SELECT
          e.platform
        FROM {hosting_devshop_project_environment} e
        WHERE project_nid = :nid
        ", array(
      ':nid' => $form_state['values']['nid']
    ))->fetchCol();

    foreach ($environments as $platform_nid) {
      $platform = node_load($platform_nid);
      $platform->publish_path = $form_state['values']['project']['code_path'] . DIRECTORY_SEPARATOR . $form_state['values']['name'] . DIRECTORY_SEPARATOR . $form_state['values']['project']['drupal_path'];
      node_save($platform);
    }
    $count = count($environments);
    drupal_set_message(t('You changed the web document root to %root. %count updated with the new information.', array(
      '%root' => $form_state['values']['project']['drupal_path'],
      '%count' => format_plural($count, t('1 site was'), t( '@count sites were', array('@count' => $count))),
    )), 'warning');
  }
}
