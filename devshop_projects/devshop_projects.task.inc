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
 * This function checks to see if there if there are any more objects
 * to disable or delete during the 'project delete' task. The list of
 * tasks that need to be done are kept in $project_node->data. See the
 * following two function for more detail on those arrays:
 *
 *   devshop_projects_project_delete_form_submit
 *   devshop_projects_project_delete_continue
 *
 * @param $pnode
 *    The project node that is to be deleted.
 *
 */

function   devshop_projects_project_delete_queue_next_task($pnode) {

  // Assume all done
  $node_updated = FALSE;
  $nid = FALSE;

  // Keys to tasks to be performed. Objects must be processed in this
  // order due to task dependencies.
  $keys = array('site' => 'disable',
		'site' => 'delete',
		'platform' => 'delete');

  // Iterate through arrays of tasks => nids and queue one tasks that
  // is left to do

  foreach ($keys as $type => $task) {
    $key1 = 'project_delete_{$type}_{$task}';
    watchdog('devshop', "key1 = $key1"); //zzz

    if (isset($pnode->data[$key1])) {
      if (count($pnode->data[$key1]) > 0) {
	// Use the first nid in the list and stop looking for any more work
	$nid = $pnode->data[$key1][0];
	break;
      }
      else {
	watchdog('devshop', "key1 = $key1 COUNT IS ZERO!"); //zzz
      }

    }
    else {
      watchdog('devshop', "key1 = $key1 is NOT SET!"); //zzz
    }
    watchdog('devshop', "unsetting key $key1 XXX"); //zzz
    unset($pnode->data[$key1]);
    $node_updated = TRUE;
  }

  // If $nid != FALSE, then we need to act on that node. If it is FALSE,
  // then we are all done. We need to set the project node status
  // field to 0 and delete that row from the hosting_devshop_project
  // table

  if ($nid) {
    watchdog('devshop', "Project delete: scheduling taskk '%s' for nid $nid");
    hosting_add_task($nid, $task);
  }

  // If we changed the project node in any way, we need to save the changes
  if ($node_updated) {
    watchdog('devshop', "updating node. status = {$node->status}"); //zzz
    //    watchdog('devshop', "updating node. status = ${node->status}"); //zzz
    node_save($pnode);
  }

  if (!$nid) {
    watchdog('devshop', "Project delete: complete!");
    $pnode->status = 0;

    //$pnode->rid = 0;
    $node_updated = TRUE;

    // delete this project
    watchdog('devshop', "deleting project from hosting_devshop_project"); //zzz
    db_query("DELETE FROM {hosting_devshop_project} WHERE nid = %d",
	     $pnode->nid);
  }

}

/*
 * This function get called when we are in the process of deleting a
 * project and all of its related sites & platforms. It is invoked
 * by the one of the hook_post_hosting_TASK_TYPE_task() hook when
 * a site disable or site/platform delete tasks complete.
 *
 * All of the sites that we need to disable and delete and all of the
 * platforms we need to delete are kept in the project node's
 * $node->data field. There are four elements in that array:
 *
 *
 *  $project_node->  data['deleting_project']: if TRUE, we are in the process
 *      of delete this project and all sites & platforms. 
 *  $project_node->data['project_delete_site_disable']: array of site NID's
 *      that are to be disabled.
 *  $project_node->data['project_delete_site_delete']: an array of site NID's
 *     that are to be deleted once they are disabled.
 *  $project_node->data['project_delete_platform_delete']: an array of platform
 *      NID's that are to be deleted once the sites based on these platforms
 *      are disabled and deleted. 
 *
 * We first disable all of the sites, then we delete them. Next we delete
 * all of the platforms. As soon as a site or platform is deleted, we remove
 * the NID from the corresponding array. When everything is deleted, we
 * remove the rows from the object table. Hostmaster will remove them from
 * the hosting context table during the delete task.
 *
 * @param $nid
 *    The NID of the object which the task just completed on. It can be
 *    the project node (when the actual project is being deleted). The
 *    site site node (when the site was disabled or deleted), or the 
 *    platform node (when the platform was deleted).
 * @param $op
 *    The operation (task type) that just complted. Will be one of:
 *    'project delete', 'site disable', 'site delete', or
 *    'platform delete'.
 */
function devshop_projects_project_delete_continue($nid, $op) {

  if (!($node = node_load($nid))) {
    watchdog('devshop', "Delete continue: nid $nid not found");
    return;
  }

  if ($node->type == 'project') {
    devshop_projects_project_delete_queue_next_task($node);
    return;
  }
  else if (($node->type != 'site') &&
	   ($node->type == 'platform')) {
    // We only care about sites and platforms
    return;
  }

  // Does this object belong to a devshop project? If not,
  // then just ignore it.
  if (!($node->project_nid)) {
    return;
  }

  // load project node
  if (!($project_node = node_load($node->project_id))) {
    watchdog('devshop', 
	     "Delete continue: can't load project nid $node->project_nid");
    return;
  }

  if (!$project_node->data['deleting_project']) {
    watchdog('devshop', 
	     "Delete continue: delete_project flag not TRUE");
    return;
  }

  // delete this object
  db_query("DELETE FROM {hosting_devshop_project_object} " .
	   "WHERE project_nid = %d AND object_nid = %d",
	   $project_node->nid, $node->nid);

  // Remove this nid from our array of things to do
  $key1 = 'project_delete_{$node->type}_{$op}';
  $key2 = array_search($node->nid, $project_node->data[$key1]);

  if ($key2 != FALSE) {
    unset($project_node->data[$key1][$key2]);
    if (count($project_node->data[$key1][$key2]) == 0) {
      unset($project_node->data[$key1]);
    }
    node_save($project_node);
  }

  // do the next delete task
  devshop_projects_project_delete_queue_next_task($project_node);
}

/*
 * Implementation of hook_post_hosting_TASK_TYPE_task
 *
 */

function devshop_projects_post_hosting_delete_task($task, $data) {
  watchdog('devshop', "Delete post task hook called");
  devshop_projects_project_delete_continue($task->ref->nid, 'delete');

}

/*
 * Implementation of hook_post_hosting_TASK_TYPE_task
 *
 */

function devshop_projects_post_hosting_disable_task($task, $data) {
  watchdog('devshop', "Disablw post task hook called");
  devshop_projects_project_delete_continue($task->ref->nid, 'disable');
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

