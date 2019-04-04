/**
 * Start New Project:
 *
 * Step 1: Takes the entered name and creates a base url and code path from it.
 */
(function ($) {

  Drupal.devshopFilterProjectName = function (inputElement) {
    return inputElement.val().split("/").pop().replace('.git', '').replace(/[^a-z0-9]/gi, '').toLowerCase()
  }

  Drupal.devshopSetProjectName = function (inputElement) {
    var fixedProjectName =  Drupal.devshopFilterProjectName(inputElement);
    if (fixedProjectName != '') {
      $('#edit-title').val(fixedProjectName);
    }
  }

  Drupal.behaviors.devshopSourceSelect = {
    attach: function (context, settings) {
      Drupal.settings.devshop.projectNameSourceElements.forEach(function( selector ) {
        console.log($(selector).prop("tagName"), selector);

        if ($(selector).prop("tagName") == 'SELECT') {
          var eventName = 'change';
        }
        else {
          var eventName = 'keyup';
        }

        $(selector).bind(eventName, function(event) {
          Drupal.devshopSetProjectName($(this));
        });
      });
    }
  };

  Drupal.behaviors.devshopComposerProjects = {
    attach: function (context, settings) {
      $('a.composer-project-link').click(function(e) {
        e.preventDefault();
        $('#edit-github-create-github-repository-source-composer-project').val($(this).html());
      });

      $('a.composer-repo-link').click(function(e) {
        e.preventDefault();
        $('#edit-github-create-github-repository-source-import').val($(this).html());
      })
    }
  }
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
