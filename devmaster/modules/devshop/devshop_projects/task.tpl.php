<div class="environment-tasks-alert alert-<?php print $task->status_class ?>" id="task-display-<?php print $task->nid; ?>">
    <div class="task-text">
          <?php foreach ($environment->domains as $domain): ?>
            <a href="<?php print 'http://' . $domain; ?>" target="_blank" class="btn btn-link"><?php print 'http://' . $domain; ?></a>
          <?php endforeach; ?>
            <i class="fa fa-<?php print $task->icon ?>"></i>
            <span class="type-name"><?php print $task->type_name ?></span>
            <span class="status-name small"><?php if ($task->task_status != HOSTING_TASK_QUEUED && $task->task_status != HOSTING_TASK_PROCESSING) print $task->status_name ?></span>
            &nbsp;
            <em class="time small"><i class="ago-icon fa fa-<?php if ($task->task_status == HOSTING_TASK_QUEUED || $task->task_status == HOSTING_TASK_PROCESSING) print 'clock-o'; else print 'calendar' ?>"></i> <time class="timeago" datetime="<?php print $task->task_timestamp ?>"><?php print $task->task_date ?></time></em>
        </a>
    </div>
    <span class="progress">
        <div class="progress-bar progress-bar-striped progress-bar-info active" role="progressbar" aria-valuenow="100" aria-valuemin="0" aria-valuemax="100">
            <span class="sr-only"></span>
        </div>
    </span>
</div>
