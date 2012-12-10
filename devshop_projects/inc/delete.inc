<?php
/**
 * @file delete.inc
 * Contains functions related to deleting a project.
 */


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

