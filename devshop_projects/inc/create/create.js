
Drupal.behaviors.createStep1 = function() {
    $( "#edit-title" ).keyup(function(event) {

      // Extract project name and base path and URL
      var projectName = $(this).val();
      var base_path = $('#edit-code-path').attr('data-base_path') + '/' + projectName;
      var base_url = projectName + '.' + $('#edit-base-url').attr('data-base_url');

      $('#edit-code-path').val(base_path);
      $('#edit-base-url').val(base_url);

    });
}