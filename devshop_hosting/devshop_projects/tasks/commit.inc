<?php
/**
 * @file commit.inc
 * Functions related to the "Commit Features" task.
 */

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Commit" task.
 */
function hosting_task_devshop_commit_form($node) {

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the environment to commit features from.'));

  $descr = 'A message describing this commit. Too see a diff output off all of the features, ';

  if (_devshop_projects_site_has_module($node, 'features_diff')) {
    $descr .= 'click ' . l(t('here.'), "http://$node->hosting_name/features/diff/all", array('absolute' => TRUE, 'attributes' => array('target' => '_blank'))) . ' Be patient. It takes a few moments for the diffs to be generatred.';
  }
  else {
    $descr .= 'enable the Features Diff module for this site, Verify the site, and select this task again.';
  }

  $form['message'] = array(
    '#title' => t('Commit Message'),
    '#type' => 'textarea',
    '#description' => $descr,
  );
  $form['push'] = array(
    '#title' => t('Push code after commit?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  $form['revert'] = array(
    '#title' => t('Force revert features after commit?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  // @TODO: Provide a DIFF display to give the user an idea of what has changed.
  return $form;
}
