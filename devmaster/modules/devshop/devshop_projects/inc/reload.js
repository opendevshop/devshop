(function ($) {
    Drupal.behaviors.devshopReload = {
        attach: function (context, settings) {
            setTimeout('Drupal.behaviors.devshopReload.checkProject()', Drupal.settings.devshopReload.delay);
        },
        checkProject: function() {
            var url = '/projects/add/status/' + Drupal.settings.devshopReload.type;
            console.log('Checking Project Status...');
            $.getJSON(url, function (data) {
                $.each(data, function (i, platform) {
                    if (platform.version) {
                        jQuery('#version-' + i).html(platform.version);
                    }
                    if (platform.profiles) {
                        jQuery('#profiles-' + i).html(platform.profiles);
                    }
                    if (platform.status) {
                        if (platform.status == 'Processing') {
                            platform.status += ' <i class="fa fa-gear fa-spin"></i>';
                        }
                        jQuery('#status-' + i).html(platform.status);
                    }
                });
                if (data.tasks_complete){
                    document.location.reload();
                } else {
                    setTimeout("Drupal.behaviors.devshopReload.checkProject()", Drupal.settings.devshopReload.delay);
                }
            });
        }
    };
}(jQuery));
