<?php
/**
 * @file pull.inc
 * Functions related to the "Pull Code" task.
 */

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Pull Code" task.
 *
 * @see drush_devshop_projects_pre_hosting_task()
 */
function hosting_task_devshop_pull_form($node) {

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the environments to pull code to. NOTE: Environments that are tracking a tag or have pull disabled are not listed.'), 'environments', 'Environments', 'checkboxes');

  // Remove any environments where pull is disabled or is tracking a tag.
  foreach ($node->project->environments as $name => $environment) {
    if (isset($form['environments']['#options'][$name]) && (empty($environment->hosting_settings['deployment']['continuous']) || in_array($environment->git_ref, $node->project->settings->git['tags']))) {
      unset($form['environments']['#options'][$name]);
    }
  }

  // If empty, send them to project page.
  if (empty($form['environments']['#options'])) {
    drupal_set_message(t('You do not have any environments with Pull Code enabled, or they are all tracking tags. !link to be able to pull code.', array('!link' => l(t('Edit the project settings'), "node/$node->nid/edit"))));
    // @TODO: Close modal window instead?
    return $form;
  }

  $form['update'] = array(
    '#title' => t('Run update.php after code pull?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );

  if (_devshop_projects_project_has_module($node, 'features')) {
    $form['revert'] = array(
      '#title' => t('Revert all features after code pull?'),
      '#type' => 'checkbox',
      '#default_value' => 1,
    );
  }
  $form['cache'] = array(
    '#title' => t('Clear cache after code pull?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );

  // Add validator for environments
  //$form['#validate'] = array('hosting_task_devshop_pull_form');
  return $form;
}

/**
 * Extra submit function for hosting_task_confirm_form()
 *
 * @see devshop_projects_form_alter().  We had to add the submit hadler there.
 */
function hosting_task_devshop_pull_form_validate($form, &$form_state) {
  $value = implode(' ', array_filter($form_state['values']['parameters']['environments']));
  form_set_value($form['parameters']['environments'], $value, $form_state);
}

