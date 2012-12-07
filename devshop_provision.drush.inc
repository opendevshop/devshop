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
  //$items['provision-devshop-create'] = array(
  //  'description' => 'Create a DevShop Project with three platforms and three sites for dev, test, and live.',
  //  'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  //  'arguments' => array(
  //    'git-url' => 'The URL of a git repo to use to create the platforms.',
  //    'path' => 'Desired project path.',
  //    'branch' => 'Branch (not yet implemented!)',
  //    'retry' => 'Retry the create.',
  //  ),
  //  'aliases' => array('create-project'),
  //);
  $items['provision-devshop-delete'] = array(
    'description' => 'Delete a DevShop Project and all associated sites and platforms.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
    ),
    'aliases' => array('delete-project'),
  );
  $items['provision-devshop-platform-create'] = array(
    'description' => 'Create a new platform and add it to an existing DevShop Project.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'platform-name' => 'The name of the platform to be created.',
      'git-url' => 'The URL of a git repo to use to create the platforms.',
      'branch' => 'Git branch to clone.',
    ),
    'aliases' => array('create-platform'),
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
 * NOTE: This ONLY runs when being called this way.
 * This hook should ONLY be used to pass Options from a hostmaster task form to 
 * the $task object.
 */
function drush_devshop_provision_pre_hosting_task() {
  $task =& drush_get_context('HOSTING_TASK');

  // Verify Platform
  // For our platforms, we have to clone it if it has a git_remote
  // If it has a git branch, we should checkout as well.
  if ($task->ref->type == 'platform' && $task->task_type == 'verify' && !empty($task->ref->git_url)) {
    
    $platform = $task->ref;
    $root = $platform->publish_path;
    $git_remote = $platform->git_url;
    $git_branch = $platform->git_branch;
    
    // Check if a repo exists
    if (!drush_shell_cd_and_exec($repo_path, 'git status')){
      drush_log(dt("[DEVSHOP] No Repo found at !root.  Cloning from !git", array('!git' => $platform->git_url, '!root' => $root)));
      
      // Build the command string
      $command = "git clone $git_remote $root";
      if ($git_branch) {
        $command .= " --branch $git_branch";
      }
    }
    // If the platform has been verified and has a branch and git url
    else {
      $root = $platform->publish_path;
      $git_remote = $platform->git_url;
      $git_branch = $platform->git_branch;
      
      // Build the command string
      $command = "git checkout $git_branch";
    }
    
    // Execute
    if (!empty($command)){
       drush_log('[DEVSHOP] Running: ' . $command);
       
      // @TODO: Create a d()->server->shell_cd_and_exec() function
      //  server->shell_exec() uses escapeshellcmd(), so we cannot cd_and_exec!
      // So instead, this is the code from d()->server->shell_exec
      // d()->server->shell_exec($cmd);
      if (provision_is_local_host(d()->server->remote_host)) {
        return drush_shell_cd_and_exec($root, escapeshellcmd($command));
      }
      else {
        return drush_shell_cd_and_exec($root, 'ssh ' . drush_get_option('ssh-options', '-o PasswordAuthentication=no') . ' %s %s', $this->script_user . '@' . d()->server->remote_host, escapeshellcmd($command));
      }
       
       $output = drush_shell_exec_output();
       drush_log('Shell Output: ' . implode("\n", $output) , 'notice');
    }
    
  }
  
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

  // Projectt Create
  if ($task->ref->type == 'project' && $task->task_type == 'devshop-create') {
    $task->options['retry'] = $task->task_args['retry'];
  }
  
  // Platform Create
  if ($task->ref->type == 'project' && $task->task_type == 'devshop-platform-create') {
    $task->options['platform-name'] = $task->task_args['platform-name'];
    $task->options['git-url'] = $task->task_args['git-url'];
    $task->options['branch'] = $task->task_args['branch'];
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
      
      // TODO: $target == 'self' when using over a remote drush alias.
      // This causes fail.  It would be pretty damn impressive to be
      // be able to drush dl module via remote alias to a devshop box.
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
