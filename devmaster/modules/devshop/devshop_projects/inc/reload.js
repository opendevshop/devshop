(function ($) {
    Drupal.behaviors.devshopReload = {
        attach: function (context, settings) {
            setTimeout('Drupal.behaviors.devshopReload.checkProject()', Drupal.settings.devshopReload.delay);
        },
        checkProject: function() {
            var url = '/projects/add/status/' + Drupal.settings.devshopReload.type;
            $.getJSON(url, function (data) {
                $.each(data.tasks, function (i, platform) {
                    console.log(platform.version, "Updating " + i);
                    if (platform.version) {
                        jQuery('#version-' + i).html(platform.version);
                    }
                    if (platform.profiles) {
                        jQuery('#profiles-' + i).html(platform.profiles);
                    }
                    if (platform.link) {
                        jQuery('#status-' + i).replaceWith(platform.link);
                    }
                });

                if (data.message) {
                    jQuery('#message').html(data.message).attr('class', 'alert alert-' + data.message_class);
                }
                if (data.button) {
                    jQuery('#button').replaceWith(data.button);
                }

                if (data.tasks_complete){
                    jQuery('#progress-indicator').hide();
                } else {
                    jQuery('#progress-indicator').show();
                }

                if (data.tasks_success){
                  document.location.reload();
                }

                setTimeout("Drupal.behaviors.devshopReload.checkProject()", Drupal.settings.devshopReload.delay);

            });
        }
    };
}(jQuery));
