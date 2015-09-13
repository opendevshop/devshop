<?php

function drush_aegir_download_pre_hosting_task()
{
    $task =& drush_get_context('HOSTING_TASK');
    if ($task->task_type != 'download') {
        return;
    }

    drush_log('[AEGIR] Download package enabled...', 'ok');

    $task->options['packages'] = $task->task_args['packages'];
    $task->options['commit'] = $task->task_args['update'];
    $task->options['message'] = $task->task_args['message'];
    $task->options['update'] = $task->task_args['update'];
}
