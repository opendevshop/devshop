<?php

/**
 * Implements drush_HOOK_pre_COMMAND()
 *
 * This runs for each tasks during the command
 *    drush @hostmaster hosting-tasks
 *
 * Passes the softlayer api username and key to the drush provision-verify
 * command as an option.
 */
function drush_devshop_softlayer_pre_hosting_task() {
  $task =& drush_get_context('HOSTING_TASK');
  if ($task->ref->type == 'server' && $task->task_type == 'verify') {
    $task->options['softlayer_api_key'] = variable_get('devshop_cloud_softlayer_api_key','YOURKEY');
    $task->options['softlayer_api_username'] = variable_get('devshop_cloud_softlayer_api_username','YOURUSERNAME');
  }
}