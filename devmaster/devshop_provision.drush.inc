<?php
/**
 * @file
 * Drush commands for DevShop Provision
 */

/**
 * Implements hook_drush_init()
 * - Ensures that provision is loaded before devshop_provision
 */
function devshop_provision_drush_init(){
  $list = drush_commandfile_list();
  $provision_drush_inc = $list['provision'];
  include_once($provision_drush_inc);
  include_once('devshop_provision.context.project.inc');
}

/**
 * Implementation of hook_drush_command().
 * Provides provision commands for all devshop tasks.
 */
function devshop_provision_drush_command() {
  $items['provision-devshop-pull'] = array(
    'description' => 'Pull & verify platform code and (optionally) run update.php, clear cache, and revert features.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'update' => 'Run update.php after code pull.',
      'revert' => 'Revert all features after code pull.',
      'cache' => 'Clear all caches after code pull.',
      'reset' => 'Runs "git reset --hard" before pulling.',
    ),
    'arguments' => array(
      'environments' => 'A list of environment names to Pull Code to.',
    ),
    'examples' => array(
      'drush @project_mysite provision-devshop-pull dev test --cache' => 'Triggers a git pull and a clear cache command on @project_mysite\'s dev and test environments.', 
    ), 
    'aliases' => array('pdp'),
  );
  $items['provision-devshop-sync'] = array(
    'description' => 'Sync database (and files, coming soon) from a chosen source site.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'update' => 'Run update.php after content sync.',
      'revert' => 'Revert all features after content sync.',
      'cache' => 'Clear all caches after content sync.',
      'files' => 'Sync site files.',
    ),
    'arguments' => array(
      'from' => 'Environment to sync from.',
      'to' => 'Environment to sync to.',
    ),
    'examples' => array(
      'drush @project_mysite provision-devshop-sync live dev' => 'Syncs the database from for project_mysite\'s live to  project_mysite\'s dev server.', 
    ), 
    'aliases' => array('pds'),
  );
  $items['provision-devshop-commit'] = array(
    'description' => 'Export the site\'s Features and commit the result.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'message' => 'Add a commit message',
      'revert' => 'Force revert all features after exporting and committing.',
    ),
    'arguments' => array(
      'environment' => 'The name of the environment to commit from.',
    ),
    'examples' => array(
      'drush @project_mysite provision-devshop-commit dev --message="changed some settings"' => 'Recreates and Commits all features from the dev environment of @project_mysite with an additional commit message.', 
    ), 
    'aliases' => array('pdc'),
  );
  /* @TODO: Make devshop dl an addon module.
  $items['provision-devshop-dl'] = array(
    'description' => 'Download modules.  This is just a wrapper for dl, it is here for hostmaster.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'modules' => 'The list of modules to DL.',
    ),
    'aliases' => array('pdl'),
  );
  */
  $items['provision-devshop-test'] = array(
    'description' => 'Run a group of SimpleTests.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'tests-to-run' => 'The list of tests to run, separated by comma.',
    ),
    'aliases' => array('pdt'),
    'arguments' => array(
      'environment' => 'The name of the environment to run tests on.',
    ),
  );
  return $items;
}

/**
 * Function for checking if this is a project and we have a repo.
 *
 * Used in pre drush command hooks
 */
function devshop_provision_pre_flight($platform_name = NULL){
  if (d()->type != 'project'){
    return drush_set_error(DEVSHOP_FRAMEWORK_ERROR, 'All provision-devshop-* commands must be run on a project alias.');
  }
}
