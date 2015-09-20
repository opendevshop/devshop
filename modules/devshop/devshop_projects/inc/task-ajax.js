
Drupal.behaviors.devshopTasks = function() {
    setTimeout("devshopCheckTasks()", 2000);
}

var devshopCheckTasks = function(){
    console.log('Checking...');

    if (Drupal.settings.devshopProject) {
        $.get('/devshop/tasks/' + Drupal.settings.devshopProject, null, devshopTasksUpdate , 'json');
    }
    else {
        $.get('/devshop/tasks', null, devshopTasksUpdate , 'json');
    }
}

var devshopTasksUpdate = function (data) {
    console.log(data);
    alert('yeah');
}