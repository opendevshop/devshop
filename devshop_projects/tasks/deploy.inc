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
    '#default_value' => arg(3) == 'ref'? arg(4): '',
  );

  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the environment to deploy the chosen git branch or tag to.'), 'environment', 'Environment', 'radios');

  $form['environment']['#default_value'] =  isset($_GET['environment'])? $_GET['environment']: '';

  // Detect pre-selected environments
  if ($_GET['environment']) {

    // @TODO: Check that Git ref exists!
    // @TODO: Display current branch!
    // @TODO: Display timestamps for the last commit in the two Git Refs!

    $form['environment']['#type'] = 'value';
    $form['environment']['#value'] =  $_GET['environment'];

    $form['git_ref']['#type'] = 'value';
    $form['git_ref']['#value'] =  $_GET['git_ref'];

    $form['environment_label'] = array(
      '#type' => 'item',
      '#title' => t('Environment'),
      '#value' => $_GET['environment'],
      '#description' => t('The environment to deploy code to.'),
      '#prefix' => '<div class="sync-envs"><div class="source">',
      '#suffix' => '</div>',
    );
    $form['git_ref_label'] = array(
      '#type' => 'item',
      '#title' => t('Git Reference'),
      '#value' => $_GET['git_ref'],
      '#description' => t("The Git branch or tag to deploy."),
      '#prefix' => '<div class="destination">',
      '#suffix' => '</div></div>',
    );
  }

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
