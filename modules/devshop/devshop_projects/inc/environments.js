
(function ($) {
  Drupal.behaviors.devshopPlatforms = {
    attach: (context, settings) {
      $('.form-option .description').hide();
      $(".form-option").hover(
        function() {
            $(this).find(".description").show();
        },
        function() {
            $(this).find(".description").hide();
        }
      );
    }
  };
}(jQuery));
