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
            n.title as project
        FROM {hosting_devshop_project_environment} h
        LEFT JOIN {node} n ON h.project_nid = n.nid

        ';

    if (!empty($project_nid)) {
        $sql .= ' WHERE project_nid = %d';
    }
    $query = db_query($sql, $project_nid);

    // Get all environments and their status.
    while ($result = db_fetch_object($query)) {
        $result->last_task = (object) unserialize($result->last_task);
        $result->last_task->status_name = devshop_task_status_class($result->last_task->status);

        $output[] = $result;
    }

    print json_encode($output);
    exit;
}