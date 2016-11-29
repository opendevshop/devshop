<?php

/**
 * Implements hook_form_alter() for Site Creation nodes.
 *
 * @see devshop_projects_form_alter()
 *
 */
function devshop_projects_create_environment_form(&$form, &$form_state, $form_id) {

  // Look for project. If there is none, return.
  $project_node = node_load(arg(3));
  if ($project_node->type != 'project') {
    return;
  }

  drupal_set_title(t('Create new Environment'));

  // Generate field prefix and suffix using domain name pattern.
  $pattern = variable_get('devshop_project_environment_url_pattern', '@environment.@project.@hostname');

  $labels = explode('@environment', strtr($pattern, array(
    '@project' => $project_node->title,
    '@hostname' => $_SERVER['SERVER_NAME'],
  )));

  $form['title']['#access'] = false;
  $form['title']['#required'] = false;
  $form['environment_name'] = array(
    '#title' => t('Environment Name'),
    '#type' => 'textfield',
    '#description' => t('Enter a system name for your environment.  For consistency, you might want to match the branch name.'),
    '#required' => TRUE,
    '#field_prefix' => '<div class="input-group"><div class="input-group-addon">http://' . $labels[0] . '</div>',
    '#field_suffix' => '<div class="input-group-addon input-group-addon-url">' . $labels[1] .'</div>
        </div>',
    '#size' => 10,
    '#maxlength' => 64,
    '#weight' => -100,
    '#attributes' => array(
      'data-placement' => 'bottom',
    ),
    '#element_validate' => array(
      'devshop_projects_create_environment_form_validate_name'
    ),
  );

  $form['project'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,
  );

}

/**
 * Validator for the domain name field: Sets domain according to pattern.
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_projects_create_environment_form_validate_name($element, &$form_state, $form) {
  if (!empty($element['#value'])) {

    // Generate field prefix and suffix using domain name pattern.
    $pattern = variable_get('devshop_project_environment_url_pattern', '@environment.@project.@hostname');
    $project = node_load($form_state['values']['project']);
    $domain = strtr($pattern, array(
      '@environment' => $element['#value'],
      '@project' => $project->title,
      '@hostname' => $_SERVER['SERVER_NAME'],
    ));
    form_set_value($form['title'], $domain, $form_state);
  }
}