(function ($) {
    Drupal.behaviors.devshopReload = {
        attach: function () {
            setTimeout("checkProject()", Drupal.settings.devshopReload.delay);
        }
    };
})(jQuery);

    var checkProject =  function() {
        console.log('Checking...');
        jQuery.get('/projects/add/status/' + Drupal.settings.devshopReload.type, null, reloadPage, 'json');
    }

    var reloadPage =  function(data){
        // Populate versions and install profiles
        jQuery.each(data.tasks, function(i, platform) {
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
            setTimeout("checkProject()", Drupal.settings.devshopReload.delay);

        }
    }
