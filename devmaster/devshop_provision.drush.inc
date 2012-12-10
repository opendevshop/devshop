<?php
/**
 * @file
 * Provision/Drush hooks for git commands.
 */

/**
 * Implements hook_drush_init()
 */
function devshop_provision_drush_init(){
  $list = drush_commandfile_list();
  $provision_drush_inc = $list['provision'];
  include_once($provision_drush_inc);
  include_once('devshop_provision.context.project.inc');
}

function devshop_provision_drush_exit(){
}

//include_once('devshop_provision.context.project.inc');

/**
 * Implementation of hook_drush_command().
 */
function devshop_provision_drush_command() {
  $items['provision-devshop-create'] = array(
    'description' => 'Create a DevShop platform and site.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'platform-name' => 'The name of the platform to be created.',
      'branch' => 'Branch',
      'pull' => 'Pull on Commit',
    ),
  );
  $items['provision-devshop-delete'] = array(
    'description' => 'Delete a DevShop Project and all associated sites and platforms.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
    ),
  );
  $items['provision-devshop-pull'] = array(
    'description' => 'Pull & verify platform code and (optionally) run update.php, clear cache, and revert features.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      //'sync' => 'Sync content from default source site before code pull.',
      'update' => 'Run update.php after code pull.',
      'revert' => 'Revert all features after code pull.',
      'cache' => 'Clear all caches after code pull.',
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
      'from' => 'Site alias to sync from.',
      'to' => 'Site alias to sync to.',
    ),
    'aliases' => array('pds'),
  );
  $items['provision-devshop-commit'] = array(
    'description' => 'Export the site\'s Features and commit the result.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'revert' => ' Force revert all features after exporting and committing.',
    ),
    'aliases' => array('pdc'),
    'arguments' => array(
      'from' => 'Site alias to sync from.',
    ),
  );
  $items['provision-devshop-dl'] = array(
    'description' => 'Download modules.  This is just a wrapper for dl, it is here for hostmaster.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'modules' => 'The list of modules to DL.',
    ),
    'aliases' => array('pdl'),
  );

  $items['provision-devshop-test'] = array(
    'description' => 'Run a group of SimpleTest tests.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'tests-to-run' => 'The list of tests to run.',
      'sync-from-live' => 'Sync contents from Live before running tests.',
    ),
    'aliases' => array('pdt'),
  );
  return $items;

}

/**
 * Implements drush_HOOK_pre_COMMAND()
 *
 * This runs for each tasks during the command
 *    drush @hostmaster hosting-tasks
 *
 * NOTE: This ONLY runs when being called from a hostmaster task.
 * This hook should ONLY be used to pass Options from a hostmaster task form to
 * the $task object, or if you don't need this functionality from the command
 * line.
 */
function drush_devshop_provision_pre_hosting_task() {
  $task =& drush_get_context('HOSTING_TASK');

  // Pull
  if ($task->ref->type == 'platform' && $task->task_type == 'devshop-pull') {
    $task->options['no-update'] = !$task->task_args['update'];
    $task->options['no-revert'] = !$task->task_args['revert'];
    $task->options['no-cache'] = !$task->task_args['cache'];
    $task->options['sites'] = explode('|', $task->task_args['sites']);
  }

  // Commit
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-commit') {
    $task->options['message'] = $task->task_args['message'];
    $task->options['push'] = $task->task_args['push'];
    $task->options['revert'] = $task->task_args['revert'];
  }

  // Sync
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-sync') {
    $task->options['source'] = $task->task_args['source'];
    $task->options['pull'] = $task->task_args['pull'];
    $task->options['no-update'] = !$task->task_args['update'];
    $task->options['no-revert'] = !$task->task_args['revert'];
    $task->options['no-cache'] = !$task->task_args['cache'];
  }

  // Platform Create
  if ($task->ref->type == 'project' && $task->task_type == 'devshop-create') {
    $task->options['platform-name'] = $task->task_args['platform_name'];
    $task->options['branch'] = $task->task_args['branch'];
    $task->options['pull'] = $task->task_args['pull'];
  }

  // Download
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-dl') {
    $task->options['modules'] = $task->task_args['modules'];
  }

  // Run tests
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-test') {
    $tests = array();

    foreach(explode("\n", $task->task_args['tests_to_run']) as $test) {
      $test = trim($test);
      if (strlen($test) > 0) {
	$tests[] = $test;
      }
    }
    $task->options['tests-to-run'] = implode($tests, ",");
    $task->options['sync-from-live'] = $task->task_args['sync'];
  }
}
