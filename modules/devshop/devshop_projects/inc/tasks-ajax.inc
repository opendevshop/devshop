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
            e.platform,
            s.status AS site_status
        FROM {hosting_devshop_project_environment} e
        LEFT JOIN {node} n ON e.project_nid = n.nid
        LEFT JOIN {hosting_site} s ON e.site = s.nid
          WHERE s.status != %d
        ';
    $args[] = HOSTING_SITE_DELETED;

    if (!empty($project_nid)) {
        $sql .= ' WHERE project_nid = %d';
        $args[] = $project_nid;
    }
    $query = db_query($sql, $args);

    // Get all environments and their status.
    while ($result = db_fetch_object($query)) {
        $result->last_task = node_load($result->last_task_nid);

        // If no last task node was found, skip.
        if (empty($result->last_task)) {
            continue;
        }
        $result->last_task->ref = node_load($result->last_task->rid);

        // If page is requesting this task's logs, load them.
        if ($_GET['task'] == $result->last_task->nid) {
            $task_node = node_load($result->last_task->nid);
            $messages = devshop_task_get_messages($task_node);
            if (count($messages)) {
                $result->last_task->logs = implode("\n", $messages);
            }
        }

        $output[] = $result;
    }

    print json_encode($output);
    exit;
}