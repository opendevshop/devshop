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
    'description' => 'Create a DevShop Project with three platforms and three sites for dev, test, and live.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'git-url' => 'The URL of a git repo to use to create the platforms.',
      'path' => 'Desired project path.',
      'branch' => 'Branch (not yet implemented!)',
    ),
    'aliases' => array('create-project'),
  );
  $items['provision-devshop-platform-create'] = array(
    'description' => 'Create a new platform and add it to an existing DevShop Project.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'git-url' => 'The URL of a git repo to use to create the platforms.',
      'branch' => 'Branch (not yet implemented!)',
    ),
    'aliases' => array('create-platform'),
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
  $items['provision-devshop-dl'] = array(
    'description' => 'Download modules.  This is just a wrapper for dl, it is here for hostmaster.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'modules' => 'The list of modules to DL.',
    ),
    'aliases' => array('pdl'),
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

  // Platform Create
  if ($task->ref->type == 'project' && $task->task_type == 'devshop-platform-create') {
    $task->options['platform-name'] = $task->task_args['platform-name'];
  }
  
  // Download
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-dl') {
    $task->options['modules'] = $task->task_args['modules'];
  }
}

/**
 * Implements hook_drush_pm_post_download() 
 *
 * Offers to git add and push the recently downloaded project.
 */ 
function devshop_provision_drush_pm_post_download($project, $release) {

  // Only act if in a provision controlled repo.
  if (!provision_git_is_repo(NULL, TRUE)){
    return;    
  } else {
    if (drush_confirm(dt('Add to git repo and push?'))){
      $target = d()->name;
      
      //Generate commit message
      $message = array();
      $message[] = "Project Added: " . $release['name'];
      $message[] = str_repeat('-', strlen($message[0]));
      $message[] = "This commit was generated by drush dl " . $project['name'];
      $message[] = str_repeat('-', strlen($message[0]));
      if (drush_get_option('message')){
        $message[] = drush_get_option('message');
      }
      $message = implode("\n", $message);
    
      $data = array(
        'message' => $message,
      );
     
      //Invoke provision-git-add to add any new files to the index
      provision_backend_invoke($target, 'provision-git-add', 
        array($project['full_project_path']), $data);
    
      //Invoke provision-git-commit
      provision_backend_invoke($target, 'provision-git-commit', array(), $data);
      
      // Push!
      provision_backend_invoke($target, 'provision-git-push');
    }
  }

}