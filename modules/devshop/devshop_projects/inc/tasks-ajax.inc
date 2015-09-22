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

    $task_types = hosting_available_tasks();

    // Get all environments and their status.
    while ($result = db_fetch_object($query)) {
        $result->last_task = (object) unserialize($result->last_task);
        $result->last_task->status_name = devshop_task_status_class($result->last_task->status);

        // Skip verify and password tasks unless they are running
//        if ($result->last_task->status != HOSTING_TASK_PROCESSING && $result->last_task->status != HOSTING_TASK_QUEUED) {
//            if ($result->last_task->type == 'verify' || $result->last_task->type == 'login-reset') {
//                continue;
//            }
//        }

        // Override "ago" text.
        $result->last_task->ago = format_interval(time() - $result->last_task->executed, 1) .' '. t('ago') . '</em>';
        if ($result->last_task->status == HOSTING_TASK_QUEUED) {
            $result->last_task->ago = t('Queued');
        }
        elseif ($result->last_task->status == HOSTING_TASK_PROCESSING) {
            $result->last_task->ago = t('Running...');
        }

        // Set task type
        $result->last_task->type_name = $task_types['site'][$result->last_task->type]['title'];

        // Get Icon
        $result->last_task->icon = devshop_task_status_icon($result->last_task->status);
        $output[] = $result;
    }

    print json_encode($output);
    exit;
}