<?php
/**
 * @file deploy.inc
 * Functions related to the "Deploy" task.
 */

///**
// * Implementation of hook_hosting_task_TASK_TYPE_form().
// *
// * For "Deploy" task.
// *
// * @see drush_devshop_projects_pre_hosting_task()
// */
//function devshop_projects_form_hosting_task_confirm_form_alter(&$form, $form_state, $form_id) {
//
//  if ($form_id == 'hosting_task_confirm_form' && $form['task']['#value'] != 'deploy') {
//    return;
//  }
//
//  // Do not affect non-project sites.
//  $node = $form_state['build_info']['args'][0];
//  if (empty($node->project)) {
//    return;
//  }
//  $project = $node->project;
//  $environment = $node->environment;
//
//  $current_ref_type_class = $environment->git_ref_type == 'tag' ? 'tag' : 'code-fork';
//
//  $form['parameters']['environment'] = array(
//    '#type' => 'item',
//    '#weight' => -1,
//    '#title' => t('Environment'),
//    '#markup' => l($environment->uri,  $environment->url, array('attributes' => array('target' => '_blank'))) . " &nbsp; <i class='fa fa-{$current_ref_type_class}'></i> " . $environment->git_ref,
//    '#description' => t('The environment URL and current git reference.'),
//  );
//
//  // Merge in the deploy hooks form.
//  $deploy_hooks_form = devshop_environment_deploy_hooks_form($project, $environment, 'deploy');
//
//  foreach (element_children($deploy_hooks_form) as $i) {
//    $form['parameters'][$i] = $deploy_hooks_form[$i];
//  }
//  return $form;
//}
