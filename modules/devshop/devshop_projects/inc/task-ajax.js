
Drupal.behaviors.devshopTasks = function() {
    setTimeout("devshopCheckTasks()", 1000);
}

var devshopCheckTasks = function(){
    $.get('/devshop/tasks', null, devshopTasksUpdate , 'json');
}

var devshopTasksUpdate = function (data) {
    //console.log(data);
    $.each(data, function(key, value){
        var id = '#' + value.project + '-' + value.name;
        var new_class = 'alert-' + value.last_task.status_class;

        var $alert_div = $('.environment-task-logs > div', id);

        // Project Node Page
        if (Drupal.settings.devshopProject) {

            // Set class of wrapper div
            $alert_div.attr('class', new_class);

            // Set or remove active class from environment div.
            if (value.last_task.status_name == 'queued' || value.last_task.status_name == 'processing') {
                $(id).addClass('active');
            }
            else {
                $(id).removeClass('active');
            }

            // Remove any status classes and add current status
            $(id).removeClass('task-queued');
            $(id).removeClass('task-processing');
            $(id).removeClass('task-success');
            $(id).removeClass('task-error');
            $(id).removeClass('task-warning');
            $(id).addClass('task-' + value.last_task.status_name);

            // Set value of label span
            $('.alert-link > span', $alert_div).html(value.last_task.type_name);

            // Set value of "ago"
            $('.alert-link > .ago', $alert_div).html(value.last_task.ago);

            // Change icon.
            $('.alert-link > .fa', $alert_div).attr('class', 'fa fa-' + value.last_task.icon);

            // Change "processing" div
        }
        // Task Node Page
        else if (Drupal.settings.devshopTask == value.last_task.nid) {
            console.log( value.last_task);
            $badge = $('.label.task-status', '#node-' + value.last_task.nid);

            // Change Badge
            var html = '<i class="fa fa-' + value.last_task.icon + '"></i> ' + value.last_task.status_name;
            $badge.html(html);

            // Change Class
            $badge.attr('class', 'label label-default task-status label-' + value.last_task.status_class);

            // @TODO:
            // Change Duration
            // Reload Logs

        }
        // Projects List Page.
        // For now this JS is only loaded on projects list page, and node pages of type project, site, and task.
        else {
            var id = '#badge-' + value.project + '-' + value.name;

            // Set class of badge
            $(id).attr('class', 'btn btn-small alert-' + value.last_task.status_class);

            // Set title
            var title = value.last_task.type_name + ': ' + value.last_task.status_class;
            $(id).attr('title', title);

            // Change icon.
            $('.fa', id).attr('class', 'fa fa-' + value.last_task.icon);
        }

        // Update global tasks list.
        var task_id = '#task-' + value.project + '-' + value.name;

        // Set class of badge
        $(task_id).attr('class', 'list-group-item list-group-item-' + value.last_task.status_class);

        // Set value of "ago"
        $('small.task-ago', task_id).html(value.last_task.ago);

        // Change icon.
        $('.fa', task_id).attr('class', 'fa fa-' + value.last_task.icon);

    });

    // Activate or de-activate global tasks icon.
    var gear_class = 'fa fa-gear';
    if ($('.list-group-item-queued', '.devshop-tasks').length) {
        gear_class += ' active-task';
    }
    if ($('.list-group-item-processing', '.devshop-tasks').length) {
        gear_class += '  active-task fa-spin';
    }

    // Set class for global gear icon.
    $('i.fa', '#navbar-main .task-list-button').attr('class', gear_class);

    // Set count
    var count = $('.list-group-item-queued', '.devshop-tasks').length + $('.list-group-item-processing', '.devshop-tasks').length;
    if (count == 0) {
      count = '';
    }
    $('.count', '.task-list-button').html(count);

    setTimeout("devshopCheckTasks()", 1000);
}