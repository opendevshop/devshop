<?php
/**
 * @file deploy.inc
 * Functions related to the "Deploy" task.
 */

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Deploy" task.
 *
 * @see drush_devshop_projects_pre_hosting_task()
 */
function hosting_task_devshop_deploy_form($node) {

  global $user;
  $project = $node->project;
  $branch_options = devshop_projects_git_ref_options($project);

  $form = array();

  $form['git_ref'] = array(
    '#title' => t('Git Branch or Tag'),
    '#description' => t('Choose the branch or tag to deploy to this environment.'),
    '#type' => 'select',
    '#options' => $branch_options,
    '#required' => TRUE,
  );

  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the environment to deploy the chosen git branch or tag to.'), 'environment', 'Environment', 'select');


  $form['update'] = array(
    '#title' => t('Run update.php after code pull?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );

  if (_devshop_projects_project_has_module($node, 'features')){
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

  return $form;
}

/**
 * @TODO:
 * 1. Save environment with new current git ref to hostmaster using post hosting task.
 *
 * To go the route of creating new platforms and migrating...
 * we will need to:
 * 2. Create a new platform with the new git_ref.
 * 3. When platform is complete, migrate the environment's site to it.
 */


