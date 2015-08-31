
Drupal.behaviors.devshopReload = function() {
  setTimeout("devshopCheckProject()", Drupal.settings.devshopReload.delay);
}

var devshopCheckProject = function(){
    console.log('Checking...');
  $.get('/hosting/projects/add/status/' + Drupal.settings.devshopReload.type, null, devshopReloadPage , 'json');
}

var devshopReloadPage = function(data){
    // Populate versions and install profiles
    $.each(data.tasks, function(i, platform) {
        if (platform.version) {
            $('#version-' + i).html(platform.version);
        }
        if (platform.profiles) {
            $('#profiles-' + i).html(platform.profiles);
        }
        if (platform.status) {
            $('#status-' + i).html(platform.status);
        }
    });
  if (data.tasks_complete){
    document.location.reload();
  } else {
    setTimeout("devshopCheckProject()", Drupal.settings.devshopReload.delay);

  }
}

//
//hostingTaskRefreshList = function() {
//  if (!Drupal.settings.hostingTaskRefresh.nid) { 
//    return null;
//  }
//
//  var hostingTaskListRefreshCallback = function(data, responseText) {
//    // If the node has been modified, reload the whole page.
//    if (Drupal.settings.hostingTaskRefresh.changed < data.changed) {
//      // only reload if there is no modal frame currently open
//      if ($(document).data('hostingOpenModalFrame') != true) {
//        document.location.reload();
//      }
//    }
//    else {
//      $(".hosting-task-list").html(data.markup);
//
//      hostingTaskBindButtons('.hosting-task-list');
//      setTimeout("hostingTaskRefreshList()", 30000);
//    }
//  }
// 
//  hostingTaskAddOverlay('#hosting-task-list');
//  $.get('/hosting/tasks/' + Drupal.settings.hostingTaskRefresh.nid + '/list', null, hostingTaskListRefreshCallback , 'json' );
//}
//
//
//function hostingTaskAddOverlay(elem) {
//  $(elem).prepend('<div class="hosting-overlay"><div class="hosting-throbber"></div></div>');
//}
//
//
//function hostingTaskRefreshQueueBlock() {
//  if (Drupal.settings.hostingTaskRefresh.queueBlock != 1) {
//    return null;
//  }
//
//  var hostingTaskQueueRefreshCallback = function(data, responseText) {
//    $("#hosting-task-queue-block").html(data.markup);
//
//    hostingTaskBindButtons('#hosting-task-queue-block');
//    setTimeout("hostingTaskRefreshQueueBlock()", 30000);
//  }
// 
//  hostingTaskAddOverlay('#hosting-task-queue-block');
//  $.get('/hosting/tasks/queue', null, hostingTaskQueueRefreshCallback , 'json' );
//}
//
//$(document).ready(function() {
//  $(document).data('hostingOpenModalFrame', false);
//  setTimeout("hostingTaskRefreshList()", 30000);
//  setTimeout("hostingTaskRefreshQueueBlock()", 30000);
//  hostingTaskBindButtons($(this));
//  $('#hosting-task-confirm-form-actions a').click(function() {
//    if (parent.Drupal.modalFrame.isOpen) {
//      setTimeout(function() { parent.Drupal.modalFrame.close({}, {}); }, 1);
//      return false;
//    }
//  });
//
//});
//
//hostingTaskBindButtons = function(elem) {
//  $('.hosting-button-dialog', elem).click(function() {
//      $(document).data('hostingOpenModalFrame', true)
//     var options = {
//        url : '/hosting/js' + $(this).attr('href'),
//        draggable : false,
//        width : 600,
//        height : 150,
//        onSubmit : function() {
//          $(document).data('hostingOpenModalFrame', false)
//          hostingTaskRefreshQueueBlock();
//          hostingTaskRefreshList();
//        }
//      }
//      Drupal.modalFrame.open(options);
//      return false;
//   });
//}
