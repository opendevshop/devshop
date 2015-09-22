
Drupal.behaviors.devshopTasks = function() {
    setTimeout("devshopCheckTasks()", 1000);
}

var devshopCheckTasks = function(){
    if (Drupal.settings.devshopProject) {
        $.get('/devshop/tasks/' + Drupal.settings.devshopProject, null, devshopTasksUpdate , 'json');
    }
    else {
        $.get('/devshop/tasks', null, devshopTasksUpdate , 'json');
    }
}

var devshopTasksUpdate = function (data) {
    //console.log(data);
    $.each(data, function(key, value){
        var id = '#' + value.project + '-' + value.name;
        var new_class = 'alert-' + value.last_task.status_name;

        var $alert_div = $('.environment-task-logs > div', id);

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
        // Projects page.
        else {
            var id = '#badge-' + value.project + '-' + value.name;

            // Set class of badge
            $(id).attr('class', 'btn btn-small alert-' + value.last_task.status_name);

            // Set title
            var title = value.last_task.type_name + ': ' + value.last_task.status_name;
            $(id).attr('title', title);

            // Change icon.
            $('.fa', id).attr('class', 'fa fa-' + value.last_task.icon);

        }

    });
    setTimeout("devshopCheckTasks()", 1000);
}