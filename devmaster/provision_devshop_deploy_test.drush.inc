<?php

/**
 * @file
 * Provision/Drush hooks for git commands.
 */

/**
 * Implementation of hook_drush_command().
 */
function provision_devshop_deploy_test_drush_command() {
  $items['provision-devshop-deploy-test'] = array(
    'description' => 'Dump the live database and import it to test, then run the deploy',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'message' => 'Dump live database, import to test, run deploy.'
    ),
    'aliases' => array('pddt'),
  );
  return $items;
}

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_devshop_deploy_test() {
  $site_path = d()->platform->root;
  $target = d()->name;
  
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO

  // Pull latest version of db from live (this also creates a backup for rollbacks)
  provision_backend_invoke('@live', 'sql-dump --result-file=~/live.sql');

  // Drop test db
  provision_backend_invoke($target, 'sql-drop');
  
  // Backport live db to test
  provision_backend_invoke($target, 'sqlc < ~/live.sql');
  // Run provision-devshop-deploy on test
  provision_backend_invoke($target, 'pdd');
}

