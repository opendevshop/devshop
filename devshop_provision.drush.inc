<?php
/**
 * @file
 * Provision/Drush hooks for git commands.
 */
//print drush_get_option('path_to_provision_inc');
// @TODO:  HOW the hell do I include provision from another drush command??

/**
 * Implements hook_drush_init()
 *
 * This is a horrible workaround for including provision.inc BEFORE our devshop_provision.context.project.inc file...
 *
 * I don't know what to do here to fix it, at the moment.
 */
function devshop_provision_drush_init(){
  $path_to_provision = drush_get_option('path_to_provision_inc');
  if (!file_exists($path_to_provision)){
    return drush_set_error(dt('Provision not found.  This is a known bug.  Please place the following in your drushrc.php file:') ."\n". '$options["path_to_provision_inc"] = "/var/aegir/.drush/provision/provision.inc";');
  }

  include_once($path_to_provision);
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
    'description' => 'Create a DevShop Project with three platforms and three sites for dev, test, and live.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'git-url' => 'The URL of a git repo to use to create the platforms.',
      'path' => 'Desired project path.',
      'branch' => 'Branch (not yet implemented!)',
    ),
    'aliases' => array('create-project'),
  );
  $items['provision-devshop-pull'] = array(
    'description' => 'Pull & verify site code and (optionally) run update.php, clear cache, and revert features.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      //'sync' => 'Sync content from default source site before code pull.',
      'no-update' => 'Run update.php after code pull.',
      'no-revert' => 'Revert all features after code pull.',
      'no-cache' => 'Clear all caches after code pull.',
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
  return $items;
}

function drush_devshop_provision_pre_hosting_task() {
  $task =& drush_get_context('HOSTING_TASK');
  // Pull
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-pull') {
    $task->options['no-update'] = !$task->task_args['update'];
    $task->options['no-revert'] = !$task->task_args['revert'];
    $task->options['no-cache'] = !$task->task_args['cache'];
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
  
}
