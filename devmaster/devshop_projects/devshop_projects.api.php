<?php
/**
 * @file
 * devshop_projects.api.php
 * Example functions for interacting with devshop.
 */

 /**
  * Implementation of hook_devshop_project_settings()
  *
  * Provides a settings form for every environment within a project settings page.
  * This allows you to save any property to site or platform nodes automatically.
  * By using the #node_type property, the chosen value will be saved to the
  * right node of the "environment", either site or platform.
  *
  * @param $project_node
  *
  * To use this hook, simply provide an array where the keys match a property
  * of the site or platform node, to be saved when the project's settings form
  * is submitted.
  *
  * 
  * 
  */
function hook_devshop_project_settings($project_node = NULL){
  $branch_options = array_combine((array) $project_node->git_branches, (array) $project_node->git_branches);
  return array(
    'git_branch' => array(
      '#title' => t('Git Branch'),
      '#node_type' => 'platform',
      '#type' => 'select',
      '#options' => $branch_options,
    ),
  );
}
