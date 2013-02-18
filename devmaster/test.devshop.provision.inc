<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_test($environment) {
  drush_log(dt('Provision DevShop Run Tests started...'));
  
  //Get site target
  $target = $environment. '.' . d()->base_url;
  $tests = drush_get_option('tests-to-run');
  drush_log(dt("Running tests $tests"), 'ok');

  provision_backend_invoke($target, 'en simpletest');
  provision_backend_invoke($target, 'test-run', array($tests));
}

