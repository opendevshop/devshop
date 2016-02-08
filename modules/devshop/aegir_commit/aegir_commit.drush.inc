<?php

function drush_aegir_commit_pre_hosting_task()
{

    $task =& drush_get_context('HOSTING_TASK');

    if ($task->task_type != 'features_update') {
        return;
    }

    $task->options['message'] = $task->task_args['message'];
    $task->options['push'] = $task->task_args['push'];
}
