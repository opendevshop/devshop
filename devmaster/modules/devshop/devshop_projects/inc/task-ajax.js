(function ($) {
  Drupal.behaviors.devshopTasks = {
      attach: function (context, settings) {
          setTimeout("Drupal.behaviors.devshopTasks.checkTasks()", 1000);
      },
      checkTasks: function () {
          
          if (Drupal.settings.nid) {
              var url = "/data/" + Drupal.settings.nid;
          }
          else {
              var url = "/data";
          }

          $.getJSON(url, function (data) {

              if (!data.environments) {
                  return;
              }
              $.each(data.environments, function (environment_name, environment) {

                  $.each(environment.tasks, function (key, task) {

                      var environment_id = "#" + task.project_name + "-" + task.environment;
                      var task_id = "#task-display-" + task.nid;
                      var new_class = "environment-tasks-alert alert-" + task.status_class;
    
                      var $alert_div = $(task_id + ".environment-tasks-alert");

                      // If $alert_div does not exist, create it.
                      if ($alert_div.length === 0) {
                          var $lastTaskWrapper = $(".last-task-alert", environment_id);
                          $lastTaskWrapper.html(task.rendered);

                          $(".environment-task-logs .tasks-wrapper", environment_id).prepend(task.rendered);

                          $alert_div = $(task_id + ".environment-tasks-alert");

                          $("time.timeago", $alert_div).timeago();
                      }

                      // Set class of wrapper div
                      $alert_div.attr("class", new_class);

                      // Set or remove active class from environment div.
                      if (task.status_class == "queued" || task.status_class == "processing") {
                          $(environment_id).addClass("active");
                      }
                      else {
                          $(environment_id).removeClass("active");
                      }

                      // Remove any status classes and add current status
                      $(environment_id).removeClass("task-queued");
                      $(environment_id).removeClass("task-processing");
                      $(environment_id).removeClass("task-success");
                      $(environment_id).removeClass("task-error");
                      $(environment_id).removeClass("task-warning");
                      $(environment_id).addClass("task-" + task.status_class);

                      // Set value of label span
                      $(".alert-link > .type-name", $alert_div).html(task.type_name);

                      // If queued or processing, make label empty.
                      if (task.status_class == "queued" || task.status_class == "processing") {
                          if (task.status_class == "queued") {
                              $(".alert-link > .status-name", $alert_div).html("");
                          }
                          else {
                              $(".alert-link > .status-name", $alert_div).html(task.status_name);
                          }
                          $(".alert-link .ago-icon", $alert_div).removeClass("fa-calendar");
                          $(".alert-link .ago-icon", $alert_div).addClass("fa-clock-o");
                      }
                      else {
                          $(".alert-link > .status-name", $alert_div).html(task.status_name);

                          $(".alert-link .ago-icon", $alert_div).removeClass("fa-clock-o");
                          $(".alert-link .ago-icon", $alert_div).addClass("fa-calendar");
                      }

                      // Set value of "ago"
                      $(".alert-link .ago", $alert_div).html(task.ago);

                      // Change icon.
                      $(".alert-link > .fa", $alert_div).attr("class", "fa fa-" + task.icon);

                      // Change href
                      $(".alert-link", $alert_div).attr("href", task.url);

                      // Change "processing" div
                  // }
                  // Task Node Page
                  if (Drupal.settings.devshopTask == task.nid) {

                      $badge = $(".label.task-status", "#node-" + task.nid);
                      
                      // Change Badge
                      var html = "<i class='fa fa-" + task.icon + "'></i> " + task.status_name;
                      $badge.html(html);
                      
                      var wasProcessing = $badge.hasClass('label-processing');
                      
                      console.log(wasProcessing, 'was processing');
                      
                      // Change Class
                      $badge.attr("class", "label label-default task-status label-" + task.status_class);

                      // Reload Logs
                      $logs = $("#task-logs", "#node-" + task.nid);
                      $logs.html(task.logs);

                      // @TODO:
                      // Change Duration
                      $(".duration .duration-text", "#node-" + task.nid).html(task.duration);


                      // If task is not processing or queued, hide follow link.
                      if (task.task_status != 0 && task.task_status != -1) {
                          // Scroll down one last time if checked.
                          if ($('#follow').prop('checked')) {
                              window.scrollTo(0, document.body.scrollHeight);
                          }
                          $('.follow-logs-checkbox').remove();
                          $('.edit-update-status').remove();
                          $('.running-indicator').remove();
                          Drupal.settings.lastTaskStopped = true;
                      }
                      else {
                          // Scroll down if follow checkbox is checked.
                          if ($("#follow").prop("checked")) {
                              window.scrollTo(0, document.body.scrollHeight);
                          }
                      }

                      // If running, set text to indicate
                      if (task.task_status === -1) {
                          $(".running-indicator .running-label").text("Processing...");
                          $(".running-indicator .fa-gear").addClass("fa-spin");
                      }

                      // If the last task was complete, and this task is complete, stop the autoloader.
                      if (wasProcessing && task.task_status !== -1) {
                          console.log('Task just stopped. Reloading.');
                          Drupal.settings.lastTaskStopped = true;
                          location.reload();
                      }
                  }

                  // Projects List Page: Update badges.
                  if ($("body.page-projects").length) {
                      var id = "#badge-" + task.project_name + "-" + task.environment;

                      // Set class of badge
                      $(id).attr("class", "btn btn-small alert-" + task.status_class);

                      // Set title
                      var title = task.type_name + ": " + task.status_class;
                      $(id).attr("title", title);

                      // Change icon.
                      $(".fa", id).attr("class", "fa fa-" + task.icon);
                  }

                  // Update global tasks list.
                  var task_id = "#task-" + task.project_name + "-" + task.environment;

                  // Set class of badge
                  $(task_id).attr("class", "list-group-item list-group-item-" + task.status_class);

                  // Set value of "ago"
                  $("small.task-ago", task_id).html(task.ago);

                  // Change icon.
                  $(".fa", task_id).attr("class", "fa fa-" + task.icon);

              });
            });

              // Activate or de-activate global tasks icon.
              var gear_class = "fa fa-gear";
              if ($(".list-group-item-queued", ".devshop-tasks").length) {
                  gear_class += " active-task";
              }
              if ($(".list-group-item-processing", ".devshop-tasks").length) {
                  gear_class += "  active-task fa-spin";
              }

              // Set class for global gear icon.
              $("i.fa", "#navbar-main .task-list-button").attr("class", gear_class);

              // Set count
              var count = $(".list-group-item-queued", ".devshop-tasks").length + $(".list-group-item-processing", ".devshop-tasks").length;
              if (count === 0) {
                  count = "";
              }
              $(".count", ".task-list-button").html(count);

              if (Drupal.settings.lastTaskStopped != true) {
                  setTimeout("Drupal.behaviors.devshopTasks.checkTasks()", 1000);
              }
          });
      }
  }

  Drupal.behaviors.taskInfoScroll = {
    attach: function (context, settings) {

        var $task_info = $("#task-info");
        if ($task_info.length == 0) {
            return;
        }
        var task_info_top = $task_info.offset().top;
        var $project_links = $("#project-environment-links");
        var project_links_top = $project_links.offset().top;

        // Check on first load.
        if (window.pageYOffset >= task_info_top){
            $task_info.addClass("task-info-fixed");
        }
        if (window.pageYOffset >= project_links_top){
            $project_links.addClass("project-environment-links-fixed");
        }

        // On scroll, check.
        window.onscroll = function() {
            if (Drupal.settings.disableScrollCheck) {
                return;
            }

            if (window.pageYOffset == 0) {
                $task_info.removeClass("task-info-fixed");
            }

            if (window.pageYOffset >= task_info_top + 50){
                $task_info.addClass("task-info-fixed");
            }
            else if ($("#follow").length && !$("#follow").prop("checked")) {
                $task_info.removeClass("task-info-fixed");
            }

            if (window.pageYOffset >= project_links_top){
                $project_links.addClass("project-environment-links-fixed");
            }
            else if (!$("#follow").prop("checked")) {
                $project_links.removeClass("project-environment-links-fixed");
            }
        }
    }
  }
}(jQuery));
