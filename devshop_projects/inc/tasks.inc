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
  $tasks['project']['devshop-create'] = array(
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
 * For "devshop-create" task for creating a platform and site.
 */
function hosting_task_devshop_create_form($node) {
  $branch_options = array_combine($node->git_branches, $node->git_branches);
  $form['branch'] = array(
    '#title' => t('Branch'),
    '#description' => t('Choose the Git branch you with to use for this new platform.  <em>Note: If you do not see all remote branches, re-verify your project.</em>'),
    '#type' => 'select',
    '#options' => $branch_options,
    '#required' => TRUE,
  );
  // @TODO: Add "create branch" functionality.
  $form['platform_name'] = array(
    '#title' => t('Platform Name'),
    '#type' => 'textfield',
    '#description' => t('Enter the system name of your platform.  For consistency, you should make this match the branch name.'),
    '#required' => TRUE,
  );
  /* @TODO: Let devshop_pull handle this.
  $form['pull'] = array(
    '#title' => t('Enable Pull on Commit'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  */
  $form['#submit'][] = 'hosting_task_devshop_create_form_submit';
  return $form;
}

/**
 * Extra submit function for hosting_task_confirm_form()
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
>>>>>>> 648db064d26aa789328bf198f3b3e2fe45fdf029
 *
 * @see devshop_projects_form_alter().  We had to add the submit hadler there.
 */
function hosting_task_devshop_create_form_submit($form, &$form_state) {

  $project = node_load($form_state['values']['nid']);
  $platform_name = $form_state['values']['parameters']['platform_name'];
  $branch = $form_state['values']['parameters']['branch'];
  $servers = hosting_get_servers('http');
  $server = variable_get('devshop_projects_default_web_server', key($servers));

  // hosting_platform fields
  $platform = new stdClass;
  $platform->title = $project->title . '_' . $platform_name;
  $platform->publish_path = $project->code_path . '/' . $platform_name;
  $platform->web_server = $server;
  $platform->git_branch = $branch;
  $platform->project = $project->title;
  $platform->environment = $platform_name;

  watchdog('debug', 'Form state: ' . print_r($form_state['values'],1));

  watchdog('debug', 'Attempting to create: ' . print_r($platform,1));

  $platform_node = _devshop_projects_node_create('platform', $platform);


}
