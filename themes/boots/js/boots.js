(function ($) {
    Drupal.behaviors.devshopTasks = {
        attach: function (context, settings) {

            // Match the height of environments so the grid doesn't break.
            $('.environment-wrapper').matchHeight();
        },
    }
})(jQuery);
