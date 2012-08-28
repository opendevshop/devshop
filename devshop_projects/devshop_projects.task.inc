<?php
/**
 * @file devshop_project.task.inc DevShop Project Task related hooks and 
 * support function include file.
 */

/**
 * Implementation of hook_hosting_tasks()
 */
function devshop_projects_hosting_tasks() {
  $tasks = array();
  $tasks['project']['devshop-create'] = array(
    'title' => t('Create Project'),
    'description' => t('Clones the repo, and creates the platforms.'),
    'provision_save' => TRUE,
  );
  $tasks['project']['devshop-install'] = array(
    'title' => t('Install Sites'),
    'description' => t('Runs drupal installation on each of the three platforms.'),
  );
  return $tasks;
}

/**
 * Implements hook_hosting_project_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For project entities.
 */
function devshop_projects_hosting_project_context_options(&$task) {
  $task->context_options['server'] = '@server_master';
  $task->context_options['project_name'] = $task->ref->title;
  $task->context_options['install_profile'] = $task->ref->install_profile;
  $task->context_options['base_url'] = $task->ref->base_url;
  $task->context_options['code_path'] = trim($task->ref->code_path, " ");
  $task->context_options['git_url'] = $task->ref->git_url;
}

/**
 * Implements hook_hosting_site_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_site_context_options(&$task) {
  $task->context_options['project'] = $task->ref->git_url;
}

/**
 * Implements hook_hosting_site_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_platform_context_options(&$task) {
  $task->context_options['project'] = $task->ref->project;
}

