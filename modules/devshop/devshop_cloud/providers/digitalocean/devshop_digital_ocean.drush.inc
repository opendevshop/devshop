<?php


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
function drush_devshop_digital_ocean_pre_hosting_task() {

  $task =& drush_get_context('HOSTING_TASK');
  drush_log('Devshop Digital Ocean Pre Task', 'ok');

  if ($task->ref->type == 'server') {
    $task->options['digital_ocean_token'] = $task->ref->digital_ocean_token;
  }
}