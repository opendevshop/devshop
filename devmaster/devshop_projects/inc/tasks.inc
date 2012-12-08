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
  $tasks['project']['verify'] = array(
    'title' => t('Verify Project'),
    'description' => t('Verifies access to the git repository, downloads branch and tag information.'),
    'provision_save' => TRUE,
    'access callback' => 'devshop_hosting_task_menu_access',
  );
  $tasks['project']['create'] = array(
    'title' => t('Create New Platform'),
    'description' => t('Creates a new platform within this project.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  $tasks['project']['delete'] = array(
    'title' => t('Delete Project'),
    'description' => t('Delete a project and all associated sites and platforms.'),
    'access callback' => 'devshop_hosting_task_menu_access',
    'dialog' => TRUE,
  );
  return $tasks;
}

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Commit" task.
 */
function hosting_task_create_form($node) {

  $form['message'] = array(
    '#title' => t('Commit Message'),
    '#type' => 'textarea',
    '#description' => $descr,
  );
  $form['push'] = array(
    '#title' => t('Push code after commit?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  $form['revert'] = array(
    '#title' => t('Force revert features after commit?'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  // @TODO: Provide a DIFF display to give the user an idea of what has changed.
  return $form;
}


/**
 * Implements hook_hosting_project_context_options()
 * 
 * This transfers data from the node to thes aegir context object (the alias!)
 * For project entities.  This is where we find the branches and tags on the remote.
 */
function devshop_projects_hosting_project_context_options(&$task) {
  
  drush_log('[DEVSHOP] devshop_projects_hosting_project_context_options()', 'ok');
  $task->context_options['server'] = '@server_master';
  $task->context_options['project_name'] = $task->ref->title;
  $task->context_options['install_profile'] = $task->ref->install_profile;
  $task->context_options['base_url'] = $task->ref->base_url;
  $task->context_options['code_path'] = trim($task->ref->code_path, " ");
  $task->context_options['git_url'] = $task->ref->git_url;
  
  $branches = getBranchesAndTags($task->ref->git_url);
  
  $task->ref->git_branches = $branches['branches'];
  $task->ref->git_tags = $branches['tags'];
  node_save($task->ref);
  
  $task->context_options['git_branches'] = $branches['branches'];
  $task->context_options['git_tags'] = $branches['tags'];
}

/**
 * Helpfer for getting branches and tags from a git URL
 */
function getBranchesAndTags($git_url = NULL){
  if (is_null($git_url)){
    $git_url = drush_get_option('git_url');
  }
  $command = "git ls-remote {$git_url}";
  drush_shell_exec($command);
  $exec_output = drush_shell_exec_output();
  
  drush_log('[DEVSHOP] running '.$command, 'ok');
  drush_log('[DEVSHOP] ' . implode("\n", $exec_output), 'ok');

  // Check for Permission Denied
  // @TODO: Provide link to the Public key for the server.
  if ('Permission denied' == substr($exec_output[0], 0, 17)){
    drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('[DEVSHOP] Error:'). implode("\n", $exec_output), 'error');
  }
  
  // If remote list is empty, something else went wrong.
  if (count($exec_output) == 1 && empty($exec_output[0])){
    drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('Something went wrong.  Check the git URL and try again.'), 'error');
    return;
  }
  
  // Build tag and branch list
  $branches = array();
  $tags = array();
  
  foreach ($exec_output AS $line_string){
    // @TODO: Would love some regex love here
    // Example remote line:
    // 9fc5727c0823d8d3300ba5aae3328d5998033e45	refs/heads/3-00
    $line = explode("	", $line_string);
    $ref = explode("/", $line[1]);
    
    $branch = array_pop($ref);
    
    if ($ref[1] == 'heads') {
      $branches[] = $branch;
    } else if ($ref[1] == 'tags') {
      $tags[] = $branch;
    }
  }
  drush_log(dt('[DEVSHOP] Found !b branches and !t tags.', array('!b' => count($branches), '!t' => count($tags), )), 'ok');
  return array('branches' => $branches, 'tags' => $tags);
}

/**
 * Implements hook_hosting_site_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_site_context_options(&$task) {
  $task->context_options['project'] = $task->ref->project;
  $task->context_options['nerd'] = 'vision';
}

/**
 * Implements hook_hosting_site_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_platform_context_options(&$task) {
  if (!empty($task->ref->project)){
    $task->context_options['project'] = $task->ref->project;
    $task->properties['task properties'] = 'works';
    $task->ref->properties['task ref properties'] = 'works';
    
    d()->setProperty('setProperty', 'works');
  }
}

/*
 * Implementation of hook_post_hosting_TASK_TYPE_task
 *
 * If a new platform has been added to a project and the env
 * is not dev, test, or live, auto-create a site....
 *
 *
 * @TODO: This is used only for creating the new site when a "branch platform" 
 * is created... we should save something in the task or platform node so we 
 * don't have to use this logic.
 */
function devshop_projects_post_hosting_verify_task($task, $data) {

  // We only case about platforms.  
  if ($task->ref->type != 'platform') {
    return;
  }

  // Get objects
  $nid = $task->ref->nid;
  $platform = node_load($nid);

  // If this platform isn't in a project, bail.
  if (empty($platform->project_nid)){
    return;
  }
  
  // Get the project
  $project = node_load($platform->project_nid);
  
  // If the project doesn't have an install profile chosen yet, bail.
  if (empty($project->install_profile)){
    return;
  }

  // If the project has a site already
  $sites = array_flip($project->project_objects['site']);
  if (isset($sites[$platform->project_environment])){
    return;
  }

  // So this is a platform which is in a project and it not dev, test, or
  // live. Let's create a site based off of this platform.
  devshop_projects_create_site($project, $platform, $platform->project_environment);  
}

function watchcat($m, $l) {
  $f = fopen("/tmp/{$m}.catlog", "a+");
  $cr = ($l[strlen($l) - 1] == '\n') ? true : false;
  fwrite($f, sprintf("%s %s%s", date('Y-m-d H:i:s'), $l, $cr ? "" : "\n"));
  fclose($f);
}


/*
 * This function checks to see if there if there are any more objects
 * to disable or delete during the 'project delete' task. The list of
 * tasks that need to be done are kept in the project data field. See the
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
  $updated = FALSE;
  $nid = FALSE;
  $data = devshop_projects_project_data_get($pnode->nid);

  // Keys to tasks to be performed. Objects must be processed in this
  // order due to task dependencies.
  $keys = array(array('type' => 'site',
		      'task' => 'disable'),
		array('type' => 'site',
		      'task' => 'delete'),
		array('type' => 'platform',
		      'task' => 'delete'));

  // Iterate through arrays of tasks => nids and queue one tasks that
  // is left to do

  foreach ($keys as $oper) {
    $key1 = "project_delete_{$oper['type']}_{$oper['task']}";

    if (isset($data[$key1])) {
      if (count($data[$key1]) > 0) {
	// Use the first nid in the list and stop looking for any more work
	$nid = $data[$key1][0];
	break;
      }
      else {
	// Array is empty, delete it
	unset($data[$key1]);
      }
      $updated = TRUE;
    }

  }

  // If $nid != FALSE, then we need to act on that node. If it is FALSE,
  // then we are all done. We need to set the project node status
  // field to 0 and delete that row from the hosting_devshop_project
  // table

  if ($nid) {
    hosting_add_task($nid, $oper['task']);
  }

  // If we changed the project data array in any way, we need to save it
  if ($updated) {
    devshop_projects_project_data_set($pnode->nid, $data);
  }

  if (!$nid) {
    $pnode->status = 0;

    //$pnode->rid = 0;
    node_save($pnode);

    // Check to see if the user wanted the project directory removed.
    // If so, make sure the directory path is valid and isn't "/"!!
    if ($data['deleting_project_remove_dir'] && 
	strlen($pnode->code_path) > 1) {
      @exec("rm -rf {$pnode->code_path}");
    }

    // delete this project
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
 *  project data['deleting_project']: if TRUE, we are in the process
 *      of delete this project and all sites & platforms. 
 *  project data['project_delete_site_disable']: array of site NID's
 *      that are to be disabled.
 *  project data['project_delete_site_delete']: an array of site NID's
 *     that are to be deleted once they are disabled.
 *  project data['project_delete_platform_delete']: an array of platform
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
 * @param $failed
 *    If TRUE, this task did not complete succesfully. We know this 
 *    the 'rollback' hook was invoked rather then the 'post task' 
 *    hook. There isn't much we can do but log it and carry on.
 */
function devshop_projects_project_delete_continue($nid, $op, $failed = FALSE) {

  if (!($node = node_load($nid))) {
    watchdog('devshop', "Delete continue: nid $nid not found");
    return;
  }

  if ($node->type == 'project') {
    // First step complete: project deleted. Now we need to
    // disable all of the sites, delete them. And then delete 
    // all of the platforms.
    devshop_projects_project_delete_queue_next_task($node);
    return;
  }
  else if (($node->type != 'site') &&
	   ($node->type != 'platform')) {
    // We only care about sites and platforms
    return;
  }


  // Does this object belong to a devshop project? If not,
  // then just ignore it.
  if (!($node->project_nid)) {
    return;
  }

  // load project node
  if (!($project_node = node_load($node->project_nid))) {
    watchdog('devshop', 
	     "Delete continue: can't load project nid $node->project_nid");
    return;
  }

  $data = devshop_projects_project_data_get($project_node->nid);

  if (!$data['deleting_project']) {
    return;
  }

  if ($op == 'delete') {
    // delete this object if the task just complete was a delete
    db_query("DELETE FROM {hosting_devshop_project_object} " .
	     "WHERE project_nid = %d AND object_nid = %d",
	     $project_node->nid, $node->nid);
  }

  // Remove this nid from our array of things to do
  $key1 = "project_delete_{$node->type}_{$op}";
  $key2 = array_search($node->nid, $data[$key1]);

  if ($key2 === FALSE) {
    watchdog('devshop', "search for nid {$node->nid} failed, key = $key2");
  }
  else {
    array_shift($data[$key1]);
    if (count($data[$key1]) == 0) {
      unset($data[$key1]);
    }
    devshop_projects_project_data_set($project_node->nid, $data);
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

/**
 * Implementation of hook_hosting_TASK_TYPE_task_rollback().
 *
 * This hook is invoked when the delete task fails. 
 * Carry on my wayward son. There will be peace when you are done. 
 * Lay your weary head to rest. Don't you cry no more.
 */
function devshop_projects_hosting_delete_task_rollback($task, $data) {
  watchdog('devshop', "Delete task rollback hook called");
  devshop_projects_project_delete_continue($task->ref->nid, 'disable', TRUE);
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task
 *
 */
function devshop_projects_post_hosting_disable_task($task, $data) {
  watchdog('devshop', "Disable post task hook called");
  devshop_projects_project_delete_continue($task->ref->nid, 'disable');
}

/**
 * Implementation of hook_hosting_TASK_TYPE_task_rollback().
 *
 * This hook is invoked when the disable task fails. But we must carry on!
 */
function devshop_projects_hosting_disable_task_rollback($task, $data) {
  watchdog('devshop', "Disable task rollback hook called");
  devshop_projects_project_delete_continue($task->ref->nid, 'disable', TRUE);
}


/**
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

