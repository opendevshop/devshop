(function ($) {
    Drupal.behaviors.devShopEnvironments = {
        attach: function (context, settings) {

            // Match the height of environments so the grid doesn't break.
            $('.environment-wrapper').matchHeight();
        },
    }
    Drupal.behaviors.timeAgo = {
        attach: function (context, settings) {
            $.timeago.settings.refreshMillis = 1000;
            $.timeago.settings.strings = {
                prefixAgo: null,
                prefixFromNow: null,
                suffixAgo: "ago",
                suffixFromNow: "from now",
                inPast: 'any moment now',
                seconds: "%d sec",
                minute: "1 min",
                minutes: "%d min",
                hour: "1 hr",
                hours: "%d hrs",
                day: "1 day",
                days: "%d days",
                month: "1 month",
                months: "%d months",
                year: "1 year",
                years: "%d years",
                wordSeparator: " ",
                numbers: []
            }

            $("time.timeago").timeago();
        }
    }
})(jQuery);
