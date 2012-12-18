

Drupal.behaviors.devshopPlatforms = function() {
  
  // DEV
  // Hide if not checked
  if (!$('#edit-default-platforms-dev-enabled').attr('checked')) {
    $('#edit-default-platforms-dev-branch-wrapper').hide();
  }
  
  // ONCLICK:
  $('#edit-default-platforms-dev-enabled').change(function(e){
    if ($(this).attr('checked')) {
      $('#edit-default-platforms-dev-branch-wrapper').show();
    } else {
      $('#edit-default-platforms-dev-branch-wrapper').hide();
    }
  });
  
  // TEST
  // Hide if not checked
  if (!$('#edit-default-platforms-test-enabled').attr('checked')) {
    $('#edit-default-platforms-test-branch-wrapper').hide();
  }
  
  // ONCLICK:
  $('#edit-default-platforms-test-enabled').change(function(e){
    if ($(this).attr('checked')) {
      $('#edit-default-platforms-test-branch-wrapper').show();
    } else {
      $('#edit-default-platforms-test-branch-wrapper').hide();
    }
  });
  
  // LIVE
  // Hide if not checked
  if (!$('#edit-default-platforms-live-enabled').attr('checked')) {
    $('#edit-default-platforms-live-branch-wrapper').hide();
  }
  
  // ONCLICK:
  $('#edit-default-platforms-live-enabled').change(function(e){
    if ($(this).attr('checked')) {
      $('#edit-default-platforms-live-branch-wrapper').show();
    } else {
      $('#edit-default-platforms-live-branch-wrapper').hide();
    }
  });
}