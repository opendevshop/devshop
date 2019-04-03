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

    Drupal.behaviors.githubSelect = {
        attach: function (context, settings) {
            $( "#edit-git-url-select").change(function(event) {
                if ($(this).val() != '_help' && $( "#edit-git-url-select" ).val() != '_custom' ) {
                    Drupal.devshopSetProjectName($(this));
                }
            });

            $( "#edit-github-repository-name").keyup(function(event) {
                if ($( "#edit-git-url-select" ).val() == '_create') {
                    Drupal.devshopSetProjectName($(this));
                }
            });
        }
    };
}(jQuery));
