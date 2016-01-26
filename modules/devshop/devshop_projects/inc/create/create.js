/**
 * Start New Project:
 *
 * Step 1: Takes the entered name and creates a base url and code path from it.
 */
(function ($) {
  Drupal.behaviors.createStep1 = {
    attach: function (context, settings) {

      $( "#edit-title" ).keyup(function(event) {

        // Extract project name and base path and URL
        var projectName = $(this).val();
        var base_path = $('#edit-code-path').attr('data-base_path') + '/' + projectName;
        var base_url = projectName + '.' + $('#edit-base-url').attr('data-base_url');

        $('#edit-code-path').val(base_path);
        $('#edit-base-url').val(base_url);

      });

    }
  };
}(jQuery))

/**
 * Step 2: Settings
 */
(function ($) {
  Drupal.behaviors.createStep2 = {
    attach: function (context, settings) {

      // Hide unless
      $('#edit-project-settings-live-live-domain-www-wrapper').hide();
      $('#edit-project-settings-live-environment-aliases-wrapper').hide();

      $('#edit-project-settings-live-live-domain').keyup(function(){
        if ($(this).val()){
          $('#edit-project-settings-live-live-domain-www-wrapper').show();
          $('#edit-project-settings-live-environment-aliases-wrapper').show();
        }
        else {
          $('#edit-project-settings-live-live-domain-www-wrapper').hide();
          $('#edit-project-settings-live-environment-aliases-wrapper').hide();
        }
      });

    }
  };
}(jQuery))
