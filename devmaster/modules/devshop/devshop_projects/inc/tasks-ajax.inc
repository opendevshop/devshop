<?php

/**
 * Page callback that returns current tasks status.
 *
 * @param string $project_name
 */
function devshop_projects_tasks_status_json() {

  $output = array();

  $sql = '
        SELECT
            last_task
        FROM {hosting_devshop_project_environment} e
        LEFT JOIN {node} n ON e.project_nid = n.nid
        LEFT JOIN {hosting_site} s ON e.site = s.nid
          WHERE n.status = 1
          AND s.status != -2
        ';

  $results = db_query($sql);

  $nids = array();

  // Ensure we always load our task
  if (isset($_GET['task'])) {
    $nids[] = $_GET['task'];
  }

  // Get all environments and their status.
  foreach ($results as $result) {

    // Save NIDs for later
    $nids[] = $result->last_task;
  }

  // Look up any remaining queued or processing tasks.
  $query = db_select('node', 'n');
  $query->join('hosting_task', 't', 'n.vid=t.vid');
  $query
    ->fields('n', array('nid'))
    ->condition('type', 'task')
    ->orderBy('n.vid', 'DESC')
    ->addTag('node_access');

  $or = db_or();
  $or->condition('task_status', HOSTING_TASK_PROCESSING);
  $or->condition('task_status', HOSTING_TASK_QUEUED);
  $query->condition($or);

  if (!empty($nids)) {
    $query->condition('n.nid', $nids, 'NOT IN');
  }
  $extras = $query->execute();
  foreach ($extras as $row) {
    $nids[] = $row->nid;
  }

  foreach (node_load_multiple($nids) as $task_node) {
    if (isset($task_node->site)) {
      $task_node->site_node = node_load($task_node->site);

      if ($task_node->site_node->site_status == HOSTING_SITE_DELETED) {
        continue;
      }
    }

    // If no last task node was found, skip.
    if (empty($task_node)) {
      continue;
    }

    // If page is requesting this task's logs, load them.
    if (isset($_GET['task']) && $_GET['task'] == $task_node->nid) {
      $messages = devshop_task_get_messages($task_node);
      if (count($messages)) {
        $task_node->logs = implode("\n", $messages);
      }
    }

    // Output a rendered task node
    $task_node->rendered = theme('devshop_task', array('task' => $task_node));
  }

  print json_encode($output);
  exit;
}
