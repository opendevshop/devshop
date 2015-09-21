
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
    $.each(data, function(key, value){
        var id = '#' + value.project + '-' + value.name;
        var new_class = 'alert-' + value.last_task.status_name;

        var $alert_div = $('.environment-task-logs > div', id);
        if (!$alert_div.hasClass(new_class)) {
            $alert_div.attr('class', new_class);
        }
    });
    setTimeout("devshopCheckTasks()", 1000);
}