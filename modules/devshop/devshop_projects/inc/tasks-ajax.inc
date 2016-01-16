<?php

/**
 * Page callback that returns current tasks status.
 *
 * @param string $project_name
 */
function devshop_projects_tasks_status_json($project_nid = '') {

    $output = array();
    $args = array();

    $sql = '
        SELECT
            project_nid,
            name,
            last_task AS last_task_nid,
            n.title as project,
            site,
            e.platform
        FROM {hosting_devshop_project_environment} e
        LEFT JOIN {node} n ON e.project_nid = n.nid
          WHERE n.status = 1
        ';

    if (!empty($project_nid)) {
        $sql .= ' WHERE project_nid = %d';
        $args[] = $project_nid;
    }
    $query = db_query($sql, $args);

    // Get all environments and their status.
    while ($result = db_fetch_object($query)) {
        $result->last_task = node_load($result->last_task_nid);

        if ($result->site) {
          $result->site_node = node_load($result->site);

          if ($result->site_node->site_status == HOSTING_SITE_DELETED) {
            continue;
          }
        }

        // If no last task node was found, skip.
        if (empty($result->last_task)) {
            continue;
        }

        // If page is requesting this task's logs, load them.
        if ($_GET['task'] == $result->last_task->nid) {
            $messages = devshop_task_get_messages($result->last_task);
            if (count($messages)) {
                $result->last_task->logs = implode("\n", $messages);
            }
        }

        $output[] = $result;
    }

  //

    print json_encode($output);
    exit;
}