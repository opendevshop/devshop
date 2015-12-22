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
            last_task AS last_task_nid,
            n.title as project,
            site,
            platform
        FROM {hosting_devshop_project_environment} h
        LEFT JOIN {node} n ON h.project_nid = n.nid

        ';

    if (!empty($project_nid)) {
        $sql .= ' WHERE project_nid = %d';
    }
    $query = db_query($sql, $project_nid);

    // Load up Hosting task language.
    $task_types = hosting_available_tasks();
    $task_types = array_merge($task_types['site'], $task_types['platform']);

    // Get all environments and their status.
    while ($result = db_fetch_object($query)) {
        $result->last_task = node_load($result->last_task_nid);
        $result->last_task->ref = node_load($result->last_task->rid);

        // In Aegir, "executed" and "delta" does not get updated properly.
        // Handle "delta" being not computed yet
        if ($result->last_task->task_status == HOSTING_TASK_PROCESSING) {
            $result->last_task->duration = format_interval(time() - $result->last_task->executed, 1);
        }
        elseif ($result->last_task->task_status == HOSTING_TASK_QUEUED) {
            $result->last_task->duration = t('Queued for %time', array('%time' => format_interval(time() - $result->last_task->changed)));
        }
        else  {
            $result->last_task->duration = format_interval($result->last_task->delta, 1);
        }

        // To test out constantly changing tasks, uncomment this line.
//        $result->last_task->task_status = rand(-1, 3);
        $result->last_task->status_class = devshop_task_status_class($result->last_task->task_status);

        $result->last_task->status_name = _hosting_parse_error_code($result->last_task->task_status);

        // Override "ago" text.
        $result->last_task->ago = format_interval(time() - $result->last_task->executed, 1) .' '. t('ago');
        if ($result->last_task->task_status == HOSTING_TASK_QUEUED) {
            $result->last_task->ago = t('Queued');
        }
        elseif ($result->last_task->task_status == HOSTING_TASK_PROCESSING) {
            $result->last_task->ago = t('Running...');
        }

        // Set task type
        $result->last_task->type_name = $task_types[$result->last_task->task_type]['title'];

        // If platform's first verify...
        if ($result->last_task->ref->type == 'platform' && $result->last_task->ref->verified == 0) {
            $result->last_task->type_name = t('Cloning Codebase');
        }

        // Get Icon
        $result->last_task->icon = devshop_task_status_icon($result->last_task->task_status);

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