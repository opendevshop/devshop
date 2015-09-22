
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

        // Set class of wrapper div
        $alert_div.attr('class', new_class);

        // Set or remove active class from environment div.
        if (value.last_task.status_name == 'queued' || value.last_task.status_name == 'processing') {
            $(id).addClass('active');
        }
        else {
            $(id).removeClass('active');
        }

        // Set value of label span
        $('.alert-link > span', $alert_div).html(value.last_task.type);

        // Set value of "ago"
        $('.alert-link > .ago', $alert_div).html(value.last_task.ago);

        // Change icon.
        $('.alert-link > .fa', $alert_div).attr('class', 'fa fa-' + value.last_task.icon);

        // Change "processing" div

    });
    setTimeout("devshopCheckTasks()", 1000);
}