

    setInterval(function(){
        if (Drupal.settings.test_running == 'FALSE') {
            return;
        }

        $.get(Drupal.settings.test_status_url, function( running ) {
          if (running == 'TRUE') {

              console.log('Still running...');
              $('.results-wrapper').load(Drupal.settings.test_url, function () {

                  if ($('#follow').prop('checked')) {
                      console.log('checked!');
                      var position = $('footer').position();
                      window.scrollTo(position.bottom,document.body.scrollHeight);
                  }
                  console.log('not checked!');

              });
          }
          else {
              console.log('Test Ended!');
              Drupal.settings.test_running = 'FALSE';
              $('.follow-checkbox ').hide();
          }
        });
    }, 1000);

