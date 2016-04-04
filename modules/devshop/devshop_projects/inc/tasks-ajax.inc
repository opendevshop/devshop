<?php

/**
 * Page callback that returns current tasks status.
 *
 * @param string $project_name
 */
function devshop_projects_tasks_status_json($project_nid = '') {

  $output = array();

  $sql = '
        SELECT
            project_nid,
            name,
            last_task,
            n.title as project,
            site,
            e.platform
        FROM {hosting_devshop_project_environment} e
        LEFT JOIN {node} n ON e.project_nid = n.nid
          WHERE n.status = 1
        ';

  if (!empty($project_nid)) {
    $sql .= ' WHERE project_nid = :project';
  }
  $results = db_query($sql, array(
    ':project' => $project_nid,
    ':status' => HOSTING_SITE_DELETED,
  ));

  $task_types = hosting_available_tasks();

  // Get all environments and their status.
  foreach ($results as $result) {
    $result->last_task_node = node_load($result->last_task);

        if ($result->site) {
          $result->site_node = node_load($result->site);

          if ($result->site_node->site_status == HOSTING_SITE_DELETED) {
            continue;
          }
        }

        // If no last task node was found, skip.
        if (empty($result->last_task_node)) {
            continue;
        }

        // If page is requesting this task's logs, load them.
        if (isset($_GET['task']) && $_GET['task'] == $result->last_task_node->nid) {
            $messages = devshop_task_get_messages($result->last_task_node);
            if (count($messages)) {
                $result->last_task_node->logs = implode("\n", $messages);
            }
        }

        $output[] = $result;
  }

  print json_encode($output);
  exit;
}
