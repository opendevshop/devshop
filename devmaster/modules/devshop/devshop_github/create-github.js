/**
 * Start New Project:
 *
 * Step 1: Takes the entered name and creates a base url and code path from it.
 */
(function ($) {
    Drupal.behaviors.githubSelect = {
        attach: function (context, settings) {
            $( "#edit-git-url-select").change(function(event) {

                if ($( "#edit-git-url-select" ).val() != '_help' && $( "#edit-git-url-select" ).val() != '_custom' ) {
                    var fixedProjectName = $(this).val().split("/").pop().replace('.git', '').replace(/[^a-z0-9]/gi, '').toLowerCase();
                    if (fixedProjectName != '') {
                        $('#edit-title').val(fixedProjectName);
                    }
                }
            });
        }
    };
}(jQuery));
