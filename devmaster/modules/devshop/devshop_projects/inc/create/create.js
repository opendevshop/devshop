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

  Drupal.behaviors.devshopShowSelect = {
    attach: function (context, settings) {
      $('a.put-back-suggested-projects').click(function(e) {
        e.preventDefault();
        var optionVal = $('#edit-settings-github-repository-source-composer-project-suggestions option').val();
        $('#edit-settings-github-repository-source-composer-project-suggestions').val(optionVal).change();
      });
    }
  }
  Drupal.behaviors.devshopShowSelectRepos = {
    attach: function (context, settings) {
      $('a.put-back-suggested-repos').click(function(e) {
        e.preventDefault();
        var optionVal = $('#edit-settings-github-repository-source-import-suggestions option').val();
        $('#edit-settings-github-repository-source-import-suggestions').val(optionVal).change();
      });
    }
  }

  Drupal.behaviors.autoSelect = {
    attach: function (context, settings) {
      $('select#edit-settings-github-repository-source-composer-project-suggestions').change(function(e) {
      if ($(this).val() == 'custom') {
        $('#edit-settings-github-repository-source-composer-project').select();
      }

      });
      $('select#edit-settings-github-repository-source-import-suggestions').change(function(e) {
      if ($(this).val() == 'custom') {
        $('#edit-settings-github-repository-source-import').select();
      }

      });
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
