<?php
/**
 * @file devshop_projects.drush.inc
 * Connects DevShop front-end to provision backend.
 */

include 'drush/contexts.inc';

/**
 * Implements drush_HOOK_pre_COMMAND()
 *
 * This runs for each task during the command
 *    drush @hostmaster hosting-tasks
 *
 * NOTE: This ONLY runs when being called from a hostmaster task.
 * This hook should ONLY be used to pass options from a hostmaster task form to
 * the $task object, or if you don't need this functionality from the command
 * line.
 */
function drush_devshop_projects_pre_hosting_task() {

  drush_log(dt("[DEVSHOP] Hello. Pre-Task check..."));

  $task =& drush_get_context('HOSTING_TASK');

  // Verify Platform
  // Here is where we hook in and clone the site
  // @TODO: Backend tasks should be able to do this.
  // @TODO: Build a "devshop-create-environment" drush command.
  if ($task->ref->type == 'platform' && $task->task_type == 'verify' && !empty($task->ref->project->git_url)) {
    drush_devshop_platform_verify();
  }

  // Pull
  if ($task->ref->type == 'project' && $task->task_type == 'devshop-pull') {
    $task->args['environments'] = $task->task_args['environments'];
    $task->options['update'] = $task->task_args['update'];
    $task->options['revert'] = !empty($task->task_args['revert']);
    $task->options['cache'] = $task->task_args['cache'];
    $task->options['force'] = FALSE;
  }

  // Deploy
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-deploy') {
    $task->args['git_ref'] = $task->task_args['git_ref'];

    // @TODO: Convert this to something more robust.
    $task->options['update'] = $task->task_args['update'];
    $task->options['revert'] = !empty($task->task_args['revert']);
    $task->options['cache'] = $task->task_args['cache'];
    $task->options['force'] = FALSE;
    $task->options['test'] = $task->task_args['test'];
  }

  // Commit
  if ($task->ref->type == 'project' && $task->task_type == 'devshop-commit') {
    $task->args['environment'] = $task->task_args['environment'];
    $task->options['message'] = $task->task_args['message'];
    $task->options['push'] = $task->task_args['push'];
    $task->options['revert'] = $task->task_args['revert'];
  }

  // Download
  if ($task->ref->type == 'project' && $task->task_type == 'devshop-dl') {
    $task->options['modules'] = $task->task_args['modules'];
  }

  // Run tests
  if ($task->ref->type == 'site' && $task->task_type == 'test') {
    $task->options['tests-to-run'] = $task->task_args['tests_to_run'];
    $task->options['test-type'] = $task->task_args['test_type'];
    $task->options['behat-folder-path'] = $task->task_args['behat_folder_path'];
    $task->options['behat-bin-path'] = $task->task_args['behat_bin_path'];

    $path = variable_get('file_directory_path', '') . '/tests';
    if (!file_exists($path)) {
      drush_log("Creating $path", 'ok');
      mkdir($path);
    }
    $task->options['output-path'] = tempnam($path, 'test-results.') . '.txt';

    $task->task_args['output_path'] = $task->options['output-path'];

    $task->no_verify = TRUE;
    node_save($task);
  }
}

/**
 * Pre hosting task hook for "verify platform" task.
 *  - Clones the repository on first run, checks out the selected after that.
 *
 * Not a hook! Called from drush_devshop_projects_pre_hosting_task
 */
function drush_devshop_platform_verify(){

  drush_log(dt("[DEVSHOP] Hello. Verifying Platform..."));

  // Verify Platform
  $task =& drush_get_context('HOSTING_TASK');

  $platform = $task->ref;
  $root = $platform->publish_path;
  $git_ref = $platform->environment->git_ref;

  $git_remote = $platform->project->git_url;
  $drupal_path = $platform->project->drupal_path;

  $create_branch = FALSE;

  $output = '';
  $commands = array();

  //Remove drupal_path to clone.
  if ($drupal_path) {
    $root = str_replace($drupal_path, '', $root);
  }

  // If there is a branch to fork, set the branch_to_fork as git_ref
  if (!empty($platform->environment->settings->branch_to_fork)) {
    $git_ref = $platform->environment->settings->branch_to_fork;
    $create_branch = TRUE;
  }

  // Check if the platform code exists. If it doesn't, clone it.
  if (!is_dir($root)) {
    drush_log(dt("[DEVSHOP] No Repo found at !root.  Cloning from !git", array('!git' => $git_remote, '!root' => $root)));

    // Build the clone command string
    $commands[] = "git clone --recursive $git_remote $root";

    // If a branch is selected, add --branch to the command.
    // @TODO: Won't the platform ALWAYS have a git_branch?
    if ($git_ref) {
      $commands[] = "git checkout $git_ref";
    }
  }
  // If the platform has been verified and has a branch and git url
  else {
    drush_log(dt("[DEVSHOP] Existing Repo found at !root.  Checking out branch !branch", array('!branch' => $git_ref, '!root' => $root)));

    // Run git fetch to ensure we have all branches and tags
    $output .= _devshop_projects_git_execute('git fetch', $root);

    // Build the command string
    $commands[] = "git checkout $git_ref";
  }

  // Execute clone or checkout.
  $output .=  _devshop_projects_git_execute($commands, $root);

  // If branch needs to be created (for "fork environment")
  if ($create_branch) {
    $commands = array();
    // Checkout the new branch.
    $commands[] = "git checkout -b {$platform->environment->git_ref}";

    // Push the branch
    $commands[] = "git push -u origin {$platform->environment->git_ref}";
    $output .=  _devshop_projects_git_execute($commands, $root);
  }

  return $output;
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for delete tasks.
 */
function devshop_projects_post_hosting_delete_task($task, $data) {

  // When a project is deleted...
  if ($task->ref->type == 'project') {
    // Queue site deletion for each environment.
    $project = $task->ref->project;
    foreach ($project->environments as $environment){
      hosting_add_task($environment->site, 'delete');
    }

    // @TODO: Should probably add our own status column
    // Unpublish the project node.
    $task->ref->status = 0;
    $task->ref->no_verify = TRUE;
    node_save($task->ref);
  }
  
  // When a site is deleted (if it is in a project, delete the platform it is on.
  // @TODO: Check for another site on this platform?
  if ($task->ref->type == 'site' && !empty($task->ref->project)) {
    // We trigger platform deletion here.
    hosting_add_task($task->ref->platform, 'delete');
  }

  // When a platform is deleted (if it is in a project), delete the environment record.
  if ($task->ref->type == 'platform' && !empty($task->ref->project)) {
    db_query('DELETE FROM {hosting_devshop_project_environment} WHERE project_nid = %d AND platform = %d', $task->ref->project->nid, $task->ref->platform);

    // If drupal root is not repo root, delete folder at code_path.
    $repo_path = str_replace($task->ref->project->drupal_path, '', $task->ref->publish_path);
    if (file_exists($repo_path) ) {
      _provision_recursive_delete($repo_path);
    }
  }
  
  // When a platform is deleted, if it is the last in the project,
  // and the project has been unpublished, delete the directory.
  if ($task->ref->type == 'platform' && !empty($task->ref->project)) {
    if ($task->ref->project->status == 0 && count($task->ref->project->environments) == 1){
      // Delete the project folder.
      exec("rm -rf {$project->code_path}");
    }
  }
}


/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for devshop deploy tasks.
 */
function devshop_projects_post_hosting_devshop_deploy_task($task, $data) {

  // Save the deployed git ref to the environment record.
  // Doing this post deploy ensures it was actually checked out.
  drush_log('[DEVSHOP] Environment Deployed.  Saving new git ref to project settings.');

  $git_ref = $task->task_args['git_ref'];

  $return = db_query('UPDATE {hosting_devshop_project_environment} SET git_ref = "%s" WHERE project_nid = %d AND name = "%s"', $git_ref, $task->ref->environment->project_nid, $task->ref->environment->name);

  if ($return) {
    drush_log("[DEVSHOP] Git reference {$git_ref} saved to {$task->ref->environment->name} environment record.");
  }
  else {
    return drush_set_error('[DEVSHOP] Environment update failed!');
  }

  // Queue up a verification of the platform.
  hosting_add_task($task->ref->environment->platform, 'verify');
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 *
 * Acts after the successful verification of platforms.
 * This is where the sites get automatically created.
 */
function devshop_projects_post_hosting_verify_task($task, $data) {

  // If this is a new platform created by a project, we will create a site here.
  if ($task->ref->type == 'platform') {
    drush_log('[DEVSHOP] Platform Verification complete.', 'notice');
    
    // If this platform isn't in a project, bail.
    $platform = $task->ref;
    if (empty($platform->project)){
      drush_log('[DEVSHOP] No project found for this platform.', 'notice');
      return;
    }

    // If the project doesn't have an install profile chosen yet, bail.
    if (empty($platform->project->install_profile)){
      drush_log('[DEVSHOP] No install profile found for this platform\'s project.', 'notice');
      return;
    }

    // If the platform has a site already, trigger verification, then bail.
    if ($platform->environment->site){
      drush_log('[DEVSHOP] Platform already has a site.', 'notice');
      // hosting_add_task($platform->environment->site, 'verify');
      return;
    }
    else {
      drush_log('[DEVSHOP] No site found for this environment. Creating...', 'notice');

      // Check for site to clone
      if ($platform->environment->settings->site_to_clone) {
        $servers = hosting_get_servers('db');

        $args['target_platform'] = $platform->nid;

        // Ensure new URI doesn't contain underscores or other invalid characters.
        $args['new_uri'] = str_replace('_', '-', $platform->environment->name) .'.'. $platform->project->base_url;

        // @TODO: Match settings of cloned environment.
        $args['new_db_server'] = key($servers);
        drush_log('[DEVSHOP] Cloning environment: ' . $platform->environment->name);

        // Queue the Cloning of the site.
        // @see devshop_projects_post_hosting_import_task().
        hosting_add_task($platform->environment->settings->site_to_clone, 'clone', $args);
      }
      else {
        devshop_projects_create_site($platform->project, $platform, $platform->environment->name);
        drush_log('[DEVSHOP] Site Created! Please wait for it to be installed. ');
      }
    }
  }
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for `clone` task.
 *
 * Ensures that cloned sites that are on project platforms inherit the project.
 */
function devshop_projects_post_hosting_clone_task($task, $data) {
  devshop_projects_post_hosting_import_task($task, $data);
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 *
 * Ensures that cloned sites that are on project platforms inherit the project.
 */
function devshop_projects_post_hosting_import_task($task, $data) {

  // If this is a new platform created by a project, we will create a site here.
  if ($task->ref->type == 'site') {
    drush_log('[DEVSHOP] Existing site has been imported!');

    $site = $task->ref;
    $platform = node_load($site->platform);

    if ($platform->environment->name) {

      // Save environment record.
      drush_log('[DEVSHOP] Project & Environment found:' . $platform->project->name . '.' . $platform->environment->name);
      $return = db_query('UPDATE {hosting_devshop_project_environment} SET site = %d WHERE project_nid = %d AND name = "%s"', $site->nid, $platform->project->nid, $platform->environment->name);
      drush_log('[DEVSHOP] Site saved to environment.');

      // Save aliases if needed.
      if ($platform->project->settings->live['environment_aliases'] && !empty($platform->project->settings->live['live_domain'])) {
        $domain =  $platform->environment->name . '.' . $platform->project->settings->live['live_domain'];
        $site->aliases = array($domain);
        drush_log('[DEVSHOP] Saving domain to the environment:' . $domain);
        node_save($site);
      }
    }
  }
}

/**
 * Utility for execute git commands.
 */
function _devshop_projects_git_execute($commands, $root) {
  // Execute
  $return = '';
  foreach ($commands as $command) {
    if (!empty($command)){
      drush_log('[DEVSHOP] Running: ' . $command);

      // @TODO: Create a d()->server->shell_cd_and_exec() function
      //  server->shell_exec() uses escapeshellcmd(), so we cannot cd_and_exec!
      // So instead, this is the code from d()->server->shell_exec
      // d()->server->shell_exec($cmd);
      if (provision_is_local_host(d()->server->remote_host)) {
        drush_shell_cd_and_exec($root, escapeshellcmd($command));
      }
      else {
        drush_shell_cd_and_exec($root, 'ssh ' . drush_get_option('ssh-options', '-o PasswordAuthentication=no') . ' %s %s', $this->script_user . '@' . d()->server->remote_host, escapeshellcmd($command));
      }

      $output = drush_shell_exec_output();
      drush_log('Shell Output: ' . implode("\n", $output) , 'warning');
      $return .= $output;
    }
  }
  return $return;
}
