

Drupal.behaviors.devshopPlatforms = function() {
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
