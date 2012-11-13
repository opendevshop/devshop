<?php



/**
 * Pre provision-devshop-deploy
 */
function drush_provision_pre_provision_devshop_run_tests(){
}


/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_test() {
  drush_log(dt('Provision DevShop Run Tests started...'));
  
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  //Get site target
  $target = d()->name;
  $site_path = d()->platform->root? d()->platform->root: d()->root;
  $destination = substr($target, 1);
  
  //Get options
  if (drush_get_option('sync-from-live')) {
    drush_log(dt("Syncing from Live"), 'ok');
    $p = strstr($destination, ".");
    if ($p) {
      $source = "live" . $p;
      $args = array('source' => $source, 'destination' => $destination);
      provision_backend_invoke($destination, 'provision-devshop-sync', $args);
    }
    else {
      return drush_set_error('DRUSH_FRAMEWORK_ERROR', 
			     dt('Unable to build source address.'));
    }
  }
  $tests = drush_get_option('tests-to-run');
  drush_log(dt("Running tests $tests"), 'ok');

  provision_backend_invoke($destination, 'test-run', array($tests));

  //Resume cron
  // @TODO: how to get hostmaster site name?  @see drush_provision_post_hostmaster_migrate()
  // they use drush_get_option('site_name')
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}

