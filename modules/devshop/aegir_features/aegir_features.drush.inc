<?php

function drush_aegir_features_pre_hosting_task()
{

    $task =& drush_get_context('HOSTING_TASK');

    drush_log(print_r($task,1 ), 'error');
    if ($task->task_type != 'features_update') {
        return;
    }

    $task->options['features'] = $task->task_args['features'];
    $task->options['revert'] = $task->task_args['revert'];
}
