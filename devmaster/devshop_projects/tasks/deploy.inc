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

  $form = array();

  $project = $node->project;
  $environment = $node->environment;

  // Detect pre-selected git ref.
  if (!empty($_GET['git_ref'])) {
    $form['git_ref'] = array(
      '#type' => 'value',
      '#value' => $_GET['git_ref'],
    );
    $form['git_ref_label'] = array(
      '#type' => 'item',
      '#title' => t('Git Reference'),
      '#value' => $_GET['git_ref'],
      '#description' => t("The Git branch or tag to deploy."),
    );
  }
  else {
    $branch_options = devshop_projects_git_ref_options($project, $environment->git_ref);
    $default_value = !empty($_GET['git_ref'])? $_GET['git_ref']: $environment->git_ref;

    $form['git_ref'] = array(
      '#title' => t('Git Branch or Tag'),
      '#description' => t('Choose the branch or tag to deploy to this environment.'),
      '#type' => 'select',
      '#options' => $branch_options,
      '#required' => TRUE,
      '#default_value' => $default_value,
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
