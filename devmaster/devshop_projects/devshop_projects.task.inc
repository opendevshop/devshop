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
  $tasks['project']['delete'] = array(
    'title' => t('Delete Project'),
    'description' => t('Delete a project and all associated sites and platforms.'),
//    'provision_save' => TRUE,
   'dialog' => TRUE,
   'access callback' => _cp_access_callback,

  );
  $tasks['project']['devshop-install'] = array(
    'title' => t('Install Sites'),
    'description' => t('Runs drupal installation on each of the three platforms.'),
  );
  $tasks['project']['devshop-platform-create'] = array(
    'title' => t('Create Platform'),
    'description' => t('Create a new platform to add to an existin project.'),
  );

  return $tasks;
}

function _cp_access_callback() {
  return TRUE;
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
  $task->context_options['retry'] = $task->ref->retry;
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


/**
 * A replacement for hosting_task_table, to allow us to add
 * tasks from other nodes.
 */
function devshop_projects_hosting_task_table($node) {
  $output = '';

  $headers[] = t('Task');
  $headers[] = array(
    'data' => t('Actions'),
    'class' => 'hosting-actions',
  );

  $tasklist = hosting_task_fetch_tasks($node->nid);
  if ($node->project_status != 'sites_ready'){
    return;
  }
  
  // Get tasklists for all sites
  $tasks = array();
  foreach ($node->project_objects['site'] as $nid => $env){
    $site_nodes[$env] = node_load($nid);
    $tasks[$env] = hosting_task_fetch_tasks($nid);
  }
  
  // Add our own specific tasks
  $tasklist['devshop-commit'] = $tasks['dev']['devshop-commit'];
  $tasklist['devshop-sync'] = $tasks['test']['devshop-sync'];
  $tasklist['devshop-pull'] = $tasks['live']['devshop-pull'];

  // Enhance titles
  $tasklist['devshop-commit']['title'] .= ' on ' . l($site_nodes['dev']->title, 'http://'. $site_nodes['dev']->title, array('attributes' => array('target' => '_blank')));
  $tasklist['devshop-sync']['title'] .= ' on ' . l($site_nodes['test']->title, 'http://'. $site_nodes['test']->title, array('attributes' => array('target' => '_blank')));
  $tasklist['devshop-pull']['title'] .= ' on ' . l($site_nodes['live']->title, 'http://'. $site_nodes['live']->title, array('attributes' => array('target' => '_blank')));

  // Override some
  unset($tasklist['devshop-create']['task_permitted']);
  unset($tasklist['devshop-create']['nid']);

  unset($tasklist['devshop-platform-create']['task_permitted']);
  unset($tasklist['devshop-platform-create']['nid']);
  
  foreach ($tasklist as $task => $info) {
    $row = array();

    if (!isset($info['nid']) && !$info['task_permitted']) {
      // just don't show those tasks, since we'll not be able to run them
      continue;
    }

    $row['type'] = array(
      'data' => $info['title'],
      'class' => 'hosting-status',
    );
    $actions = array();

    if (isset($info['task_status']) && ($info['task_status'] == 0)) {
      $actions['cancel'] = _hosting_task_button(t('Cancel'), sprintf("hosting/tasks/%d/cancel", $info['nid']), t("Cancel the task and remove it from the queue"), 'hosting-button-stop', !$info['task_permitted']);
    }
    else {
      $actions['run'] = _hosting_task_button(t('Run'), sprintf("node/%d/%s_%s", $node->nid, $node->type, $task), $info['description'], 'hosting-button-run', $info['task_permitted'], $info['dialog']);
    }

    $actions['log'] = _hosting_task_button(t('Log'), 'node/' . $info['nid'], t("Display the task log"), 'hosting-button-log', isset($info['nid']) &&  user_access('access task logs'), TRUE, FALSE);
    $row['actions'] = array(
      'data' => implode('', $actions), 
      'class' => 'hosting-actions',
    );

    $rows[] = array(
      'data' => $row,
      'class' => $info['class'],
    );
  }
  $output .= theme('table', $headers, $rows, array('class' => 'hosting-table'));
  return $output;
}

/*
 * Implementation of hook_post_hosting_TASK_TYPE_task
 *
 * If a new platform has been added to a project and the env
 * is not dev, test, or live, auto-create a site.
 */

function devshop_projects_post_hosting_verify_task($task, $data) {

  // We only case about platforms.  
  if ($task->ref->type != 'platform') {
    return;
  }

  // Get platform nid.
  $nid = $task->ref->nid;

  // Get project object information about this platform
  $p_obj = db_fetch_object(db_query("SELECT * " .
                                    "FROM {hosting_devshop_project_object} " .
                                    "WHERE object_nid = %d", $nid));

  // Not in a project
  if (!$p_obj) {
    return;
  }

  if (in_array($p_obj->env_type, array('dev', 'test', 'live'))) {
    return;
  }

  // Make sure we don't already have a site
  $s_obj = db_fetch_object(db_query("SELECT * " .
                                    "FROM {hosting_devshop_project_object} " .
                                    "WHERE object_type = 'site' AND " .
                                    "      env_type = '%s'", 
                                    $p_obj->env_type));

  // See if site exists
  if ($s_obj) {
    return;
  }

  // So this is a platform which is in a project and it not dev, test, or
  // live. Let's create a site based off of this platform.

  $platform = node_load($nid);
  $project = node_load($p_obj->project_nid);

  devshop_projects_create_site($project, $platform, $p_obj->env_type);
}

/*
 * Implementation of hook_post_hosting_TASK_TYPE_task
 *
 * This hook is invoked when a project is deleted. We then:
 *
 * 1) Disable all sites
 * 2) Delete all sites
 * 3) Delete all platforms
 * 4) Set project node status to 0
 *
 */

function devshop_projects_post_hosting_delete_task($task, $data) {

  watchdog('devshop', "Project delete post task hook called");
  // We only case about projects.  
  if ($task->ref->type != 'project') {
    watchdog('devshop', "Project delete post task hook called, but type is $task->ref->type");
    return;
  }

  // Get project nid.
  $nid = $task->ref->nid;

  // Make sure it's a real project
  $p_info = db_fetch_object(db_query("SELECT * " .
                                     "FROM {hosting_devshop_project} " .
                                     "WHERE nid = %d", $nid));
  if (!$p_info) {
    watchdog('devshop', "Project delete post task hook: project nid $nid not fund");
    return;
  }

  // First disable and delete all of the sites

  $query = db_query("SELECT * " .
		    "FROM {hosting_devshop_project_object} " .
		    "WHERE project_nid = %d AND object_type = 'site'", $nid);

  while ($site = db_fetch_object($query)) {
    hosting_add_task($site->object_nid, 'disable');
    hosting_add_task($site->object_nid, 'delete');
  }

  // Now delete all of the platforms
  $query = db_query("SELECT * " .
		    "FROM {hosting_devshop_project_object} " .
		    "WHERE project_nid = %d AND object_type = 'platform'", 
		    $nid);

  while ($platform = db_fetch_object($query)) {
    hosting_add_task($platform->object_nid, 'delete');
  }
}

/*
 * Returns TRUE is the project create task for the given node failed.
 */

function devshop_project_project_create_failed($nid, &$task) {

  if ($nid) {
    $task = hosting_get_most_recent_task($nid, 'devshop-create');
    if ($task && $task->task_status != 1) {
      // Project Create failed.
      return TRUE;
    }
  }

  // No task found OR it is successful
  return FALSE;
}

