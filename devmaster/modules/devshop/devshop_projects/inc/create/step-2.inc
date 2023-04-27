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
  if ($project->verify_task_status == HOSTING_TASK_QUEUED || $project->verify_task_status == HOSTING_TASK_PROCESSING) {
    $note = '<p class="wait">' . t('Please wait while we connect and analyze your repository.') . '</p>';
    $note .= '<p class="wait"><i class="fa fa-spinner fa-spin fa-4x text-primary"></i></p>';
    $note .= '<p class="wait">' . l(t('View Logs'), "node/$project->verify_task_nid", array(
      'attributes' => array(
        'target' => '_blank',
        'class'=> array('btn btn-primary')
      )
    ));
    $form['note'] = array(
      '#markup' => $note,
    );
    $form['#no_next'] = TRUE;

    // Add code to reload the page when complete.
    devshop_form_reloader($form, 'project');
    return $form;
  }

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

  // Add create.js script and pass an object.
  drupal_add_js(drupal_get_path('module', 'devshop_projects') . '/inc/create/create.js');
  // @TODO This is only here because create.js is currently coded to expect the array.
  // I tried, I really tried to check for this prop in create.js.
  drupal_add_js(array(
    'devshop' => array(
      'projectNameSourceElements' => array(
      ),
    ),
  ), 'setting');

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
function devshop_project_create_step_settings_submit(&$form, &$form_state) {
}
