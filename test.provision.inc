<?php

/**
 * Pre provision-devshop-test
 */
function drush_devshop_provision_pre_provision_test(){
}

/**
 * Implements the provision-devshop-test command.
 */
function drush_devshop_provision_provision_test() {
  drush_log(dt('Provision DevShop Run Tests started...'), 'status');

  // Get test info
  $tests = drush_get_option('tests-to-run');
  drush_log(dt("Running tests $tests"), 'ok');

  provision_backend_invoke('@self', 'en simpletest');
  provision_backend_invoke('@self', 'test-run', array($tests));
}

