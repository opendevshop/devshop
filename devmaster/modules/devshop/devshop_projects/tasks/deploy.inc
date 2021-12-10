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
function devshop_projects_form_hosting_task_confirm_form_alter(&$form, $form_state, $form_id) {

  if ($form_id == 'hosting_task_confirm_form' && $form['task']['#value'] != 'deploy') {
    return;
  }

  // Do not affect non-project sites.
  $node = $form_state['build_info']['args'][0];
  if (empty($node->project)) {
    return;
  }
  $project = $node->project;
  $environment = $node->environment;

  $current_ref_type_class = $environment->git_ref_type == 'tag' ? 'tag' : 'code-fork';

  $form['parameters']['environment'] = array(
    '#type' => 'item',
    '#weight' => -1,
    '#title' => t('Environment'),
    '#markup' => l($environment->uri,  $environment->url, array('attributes' => array('target' => '_blank'))) . " &nbsp; <i class='fa fa-{$current_ref_type_class}'></i> " . $environment->git_ref,
    '#description' => t('The environment URL and current git reference.'),
  );

  // Detect pre-selected git ref.
  if (!empty($_GET['git_ref'])) {

    $ref_type = $project->settings->git['refs'][$_GET['git_ref']];
    $ref_type_class = $ref_type == 'tag' ? 'tag' : 'code-fork';

    $form_params['git_ref'] = array(
      '#type' => 'value',
      '#value' => $_GET['git_ref'],
    );
    $form_params['git_ref_label'] = array(
      '#type' => 'item',
      '#title' => t('Git @ref', array('@ref' => $ref_type)),
      '#markup' => "<i class='fa fa-{$ref_type_class}'></i> " . $_GET['git_ref'],
      '#description' => t("This git reference will be checked out in this environment."),
    );
  }
  else {
    $branch_options = devshop_projects_git_ref_options($project, $environment->git_ref);
    $default_value = !empty($_GET['git_ref']) ? $_GET['git_ref'] : $environment->git_ref;

    $form_params['git_ref'] = array(
      '#title' => t('Git Branch or Tag'),
      '#description' => t('Choose the branch or tag to deploy to this environment.'),
      '#type' => 'select',
      '#options' => $branch_options,
      '#required' => TRUE,
      '#default_value' => $default_value,
    );
  }

  // Pass through a value so hostmaster knows this was triggered manually.
  // This is mainly to allow acquia cloud hooks compatibility. They have separate
  // hooks for manual deployment vs. automatic deployment.
  $form_params['manual'] = array(
    '#type' => 'value',
    '#value' => 1,
  );

  // Merge in the deploy hooks form.
  $deploy_hooks_form = devshop_environment_deploy_hooks_form($project, $environment, 'deploy');
  $form['parameters']  = array_merge($form_params, $deploy_hooks_form);

  // Merge in the deploy hooks form.
  $deploy_hooks_form = devshop_environment_deploy_hooks_form($project, $environment, 'deploy');

  foreach (element_children($deploy_hooks_form) as $i) {
    $form['parameters'][$i] = $deploy_hooks_form[$i];
  }
  return $form;
}
