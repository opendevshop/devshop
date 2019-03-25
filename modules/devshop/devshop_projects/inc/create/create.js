/**
 * Start New Project:
 *
 * Step 1: Takes the entered name and creates a base url and code path from it.
 */
(function ($) {
  Drupal.behaviors.createStep1 = {
    attach: function (context, settings) {

      // Dynamically replace special and uppercase characters.
      $( "#edit-git-url" ).keyup(function(event) {

        if ($( "#edit-title" ).val() == '') {
          var fixedProjectName = $(this).val().split("/").pop().replace(/[^a-z0-9]/gi, '').toLowerCase();
          if (fixedProjectName != '') {
            $('#edit-title').val(fixedProjectName);
          }
        }
      });

      // Dynamically replace special and uppercase characters.
      $( "#edit-title" ).keyup(function(event) {
        var fixedProjectName = $(this).val().replace(/[^a-z0-9]/gi, '').toLowerCase();
        $(this).val(fixedProjectName);

      });

    }
  };
}(jQuery));

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
}(jQuery));
