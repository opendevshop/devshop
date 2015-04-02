
    setInterval(function(){
        console.log('loading results...');
        console.log(Drupal.settings.test_url);
        if (Drupal.settings.test_url) {
            $('.results-wrapper').load(Drupal.settings.test_url, function () {
                window.scrollTo(0,document.body.scrollHeight);
            });
        }
    }, 1000);

