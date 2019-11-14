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

  $current_ref_type_class = $environment->git_ref_type == 'tag' ? 'tag' : 'code-fork';

  $form['environment'] = array(
    '#type' => 'item',
    '#title' => t('Environment'),
    '#markup' => l($environment->default_domain, 'http://' . $environment->default_domain, array('attributes' => array('target' => '_blank'))) . " &nbsp; <i class='fa fa-{$current_ref_type_class}'></i> " . $environment->git_ref,
    '#description' => t('The environment URL and current git reference.'),
  );

  // Detect pre-selected git ref.
  if (!empty($_GET['git_ref'])) {

    $ref_type = $project->settings->git['refs'][$_GET['git_ref']];
    $ref_type_class = $ref_type == 'tag' ? 'tag' : 'code-fork';

    $form['git_ref'] = array(
      '#type' => 'value',
      '#value' => $_GET['git_ref'],
    );
    $form['git_ref_label'] = array(
      '#type' => 'item',
      '#title' => t('Git @ref', array('@ref' => $ref_type)),
      '#markup' => "<i class='fa fa-{$ref_type_class}'></i> " . $_GET['git_ref'],
      '#description' => t("This git reference will be checked out in this environment."),
    );
  }
  else {
    $branch_options = devshop_projects_git_ref_options($project, $environment->git_ref);
    $default_value = !empty($_GET['git_ref']) ? $_GET['git_ref'] : $environment->git_ref;

    $form['git_ref'] = array(
      '#title' => t('Git Branch or Tag'),
      '#description' => t('Choose the branch or tag to deploy to this environment.'),
      '#type' => 'select',
      '#options' => $branch_options,
      '#required' => TRUE,
      '#default_value' => $default_value,
    );
  }

  // Merge in the deploy hooks form.
  $deploy_hooks_form = devshop_environment_deploy_hooks_form($project, $environment, 'devshop-deploy');
  $form = array_merge($form, $deploy_hooks_form);

  // Pass through a value so hostmaster knows this was triggered manually.
  // This is mainly to allow acquia cloud hooks compatibility. They have separate
  // hooks for manual deployment vs. automatic deployment.
  $form['manual'] = array(
    '#type' => 'value',
    '#value' => 1,
  );

  return $form;
}
