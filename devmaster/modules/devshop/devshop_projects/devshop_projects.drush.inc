<?php
/**
 * @file devshop_projects.drush.inc
 * Connects DevShop front-end to provision backend.
 */

include 'drush/contexts.inc';

/**
 * Implements hook_hosting_task_update_status().
 *
 * Saves task status to environment
 */
function devshop_projects_hosting_task_update_status($task, $status) {
  $ref = $task->ref;
  if (($ref->type == 'site' || $ref->type == 'platform') && isset($ref->environment)) {
    $ref->environment->last_task = $task->nid;

    // @TODO: Look for environment_target on clone task, so we can update that environment's last task as well.

    // Prevent "Verify" and "reset password" tasks from cluttering up the environment status indicator.
    // If last task is a successful verify or reset password, load the task before that as the "last task".
    if (($task->task_type == 'verify' || $task->task_type == 'login-reset') && $status == HOSTING_TASK_SUCCESS) {

      // If this object is a platform, lookup the latest tasks from the site node.
      $tasks_list = hosting_get_tasks('rid', $ref->nid);

      // The first item in $tasks should be this task.
      // If so, use the next task as the "last_task", because verify and reset password tasks are not important.
      foreach ($tasks_list as &$task_list_item) {
        // If the first task is this task, we want to skip it.
        if ($task_list_item->nid == $task->nid && $task_list_item->vid == $task->vid) {
          continue;
        }
        // If the next task is also a login-reset or verify, skip it.
        if ($task_list_item->task_type == 'verify' || $task_list_item->task_type == 'login-reset') {
          continue;
        }

        // Grab the first task that isn't this one or another skippable task
        $ref->environment->last_task = $task_list_item->nid;

        // Stop the loop through the tasks.
        break;
      }
    }

    devshop_environment_save_last_task($ref->environment);
  }
}

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
function drush_devshop_projects_pre_hosting_task()
{

  $task =& drush_get_context('HOSTING_TASK');

  // For all tasks
  if ($task->ref->type == 'site' || $task->ref->type == 'platform' || $task->ref->type == 'project') {
    $drush_aliases = new Provision_Config_ProjectAliases($task->ref->project->name, $task->ref->project);
  }
  
  // Only write drush aliases if project name was found.
  if (!empty($drush_aliases->data['name'])) {
    $drush_aliases->write();
    drush_log(dt('Drush aliases written for %name project.', [
      '%name' => $drush_aliases->data['name'],
    ]), 'devshop_log');
  }
  

  // Passing options for Download task. This should go in aegir_download.drush.inc but I can't get that hook working right now.
  if ($task->task_type == 'download') {
    drush_log('[AEGIR DEVSHOP_PROJECTS] Download package enabled...', 'ok');

    $task->options['packages'] = $task->task_args['packages'];
    $task->options['commit'] = $task->task_args['update'];
    $task->options['message'] = $task->task_args['message'];
    $task->options['update'] = $task->task_args['update'];
  }
  
  // Verify Platform
  // Here is where we hook in and clone the site
  // @TODO: This can be removed once hosting_git migration is complete.
//  if ($task->ref->type == 'platform' && $task->task_type == 'verify' && !empty($task->ref->project->git_url)) {
//    drush_devshop_platform_verify();
//   }
  
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

    // The git_ref is the only argument
    $task->args['git_ref'] = $task->task_args['git_ref'];

    // Just load in all of the task_args as options(they come from the task form.)
    foreach ($task->task_args as $arg => $value) {
      $task->options[$arg] = $value;
    }
  }

  // Run tests
  if ($task->ref->type == 'site' && $task->task_type == 'test') {

    $task->options['tests-to-run'] = $task->task_args['tests_to_run'];
    $task->options['test-type'] = $task->task_args['test_type'];
  }

  // On every task triggered by devshop, set this option.
  $task->options['is-devshop'] = TRUE;
}

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task()
 * for Verify tasks.
 *
 * Provides the "Fork environment" feature: Checks out a new branch and pushes it.
 */
function devshop_projects_post_hosting_verify_task($task, $data) {
  if ($task->ref->type == 'platform') {

    // Fork Environment:
    // If this is the first verification, and branch needs to be created (for "fork environment")
    $platform = $task->ref;
    if (!$platform->verified && !empty($platform->environment->settings->branch_to_fork)) {
      provision_process("git checkout -b {$platform->environment->git_ref}", $platform->repo_path, dt('Creating a new branch.'));

      // Push the branch
      provision_process("git push -u origin {$platform->environment->git_ref}", $platform->repo_path, dt('Pushing new branch.'));
    }
//    // Run 'composer install' if configured to do so.
//    if ($platform->environment->settings->deploy['composer'] || $platform->project->settings->deploy['default_hooks']['composer']) {
//
//      if (file_exists($platform->git['repo_path'] . '/composer.json')) {
//        $cwd = $platform->git['repo_path'];
//      }
//      elseif (file_exists($platform->publish_path . '/composer.json')) {
//        $cwd = $platform->publish_path;
//      }
//      else {
//        // If composer.json was not found in the drupal root or repo root, do not run.
//        drush_log(dt('composer.json not found, even though "composer install" hook is enabled.'), 'warning');
//        return;
//      }
////
////      provision_process("composer install", $cwd, dt('composer.json detected, running composer install in !folder', array(
////        '!folder' => $cwd,
////      )));
//
//      provision_process("git status", $platform->git['repo_path'], dt('Running git status'));
//    }
  }
}

// @TODO: Remove this function for hosting_git migration.
/**
 * Pre hosting task hook for "verify platform" task.
 *  - Clones the repository on first run, checks out the selected after that.
 *
 * Not a hook! Called from drush_devshop_projects_pre_hosting_task
 */
//function drush_devshop_platform_verify() {

//  drush_log(dt("[DEVSHOP] Hello. Preparing aegir platform codebase..."));
//
  // Verify Platform
//  $task = &drush_get_context('HOSTING_TASK');

//  $platform = $task->ref;

  // If for some reason we still do not have an environment... bail.
//  if (empty($platform->environment)) {
//    return drush_set_error('DEVSHOP_ERROR', dt('Unable to determine environment. Please contact your administrator.'));
//  }

//  $root = $platform->publish_path;
//  $git_ref = $platform->environment->git_ref;
//
//  $git_remote = $platform->project->git_url;
//  $drupal_path = $platform->project->drupal_path;
//
//  Remove drupal_path to clone.
//  if ($drupal_path) {
//    $root = str_replace($drupal_path, '', $root);
//  }

  // Check if the platform code exists. If it doesn't, clone it.
//  if (!is_dir($root)) {
//
//    // Clone the repo.
//    devshop_drush_process("git clone --recursive $git_remote $root", NULL, dt('Cloning git repository'));

    // Skip errors about submodule


    // If there is "site_to_clone", checkout that site's git ref.
    // This is done for Pull Request Environments. When a PR comes in, we want
    // to clone the site to an identical platform first. This will always succeed.
    // Then, we trigger a deployment to the new branch.
    //
    // This ensures that initial pull request environments run the same "deploy"
    // hooks. It means we don't have to hack into the "clone" task to run deploy
    // hooks anymore.
//    if ($platform->environment->settings->site_to_clone) {
//      $site = node_load($platform->environment->settings->site_to_clone);
//      $git_ref = $site->environment->git_ref;
//    }

    // If there is a git ref, check it out.
    // We don't do this on git clone, because it could be a tag.
    // @TODO: Won't the platform ALWAYS have a git_branch?
//    if ($git_ref) {
//      devshop_drush_process("git checkout $git_ref", $root, dt('Checking out specified version'));
//    }
//  }
//
//  // If this is the first verification, and branch needs to be created (for "fork environment")
//  if (!$platform->verified && !empty($platform->environment->settings->branch_to_fork)) {
//    devshop_drush_process("git checkout -b {$platform->environment->git_ref}", $root, dt('Creating a new branch.'));
//
//    // Push the branch
//    devshop_drush_process("git push -u origin {$platform->environment->git_ref}", $root, dt('Pushing new branch.'));
//  }
//
//  // Detect a bad path_to_drupal
//  if (!file_exists($platform->environment->root . '/index.php')) {
//    return drush_set_error('DEVSHOP_ERROR', dt($platform->environment->root . '/index.php file not found. Check "Path to Drupal" in Project Settings.'));
//  }
//
//  // When verifying the platform for the first time, run 'composer install' if configured to do so.
//  if ($platform->verified && $platform->environment->settings->deploy['composer'] || $platform->project->settings->deploy['default_hooks']['composer']) {
//
//    if (file_exists($platform->environment->repo_root . '/composer.json')) {
//      $cwd = $platform->environment->repo_root;
//    }
//    elseif (file_exists($root . '/composer.json')) {
//      $cwd = $root;
//    }
//    else {
//      // If composer.json was not found in the drupal root or repo root, do not run.
//      drush_log(dt('composer.json not found, even though "composer install" hook is enabled.'), 'warning');
//      return;
//    }
//
//    devshop_drush_process("composer install", $cwd, dt('composer.json detected, running composer install in !folder', array(
//      '!folder' => $cwd,
//    )));
//
//    devshop_drush_process("git status", $platform->environment->repo_root, dt('Running git status'));
//  }
//}

/**
 * Implements hook_post_hosting_TASK_TYPE_task() for delete tasks().
 */
function devshop_projects_post_hosting_delete_task($task, $data) {

  // When a project is deleted...
  if ($task->ref->type == 'project') {
    // Queue site deletion for each environment.
    $project = $task->ref->project;
    foreach ($project->environments as $environment) {
      // If site exists, trigger deletion.
      if ($environment->site) {
        hosting_add_task($environment->site, 'delete');
      }
      // If platform exists, trigger deletion.
      elseif ($environment->platform) {
        hosting_add_task($environment->platform, 'delete', array('force' => 1));
      }
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
    db_delete('hosting_devshop_project_environment')
      ->condition('project_nid', $task->ref->project->nid)
      ->condition('platform', $task->ref->platform)
      ->execute();

    // If drupal root is not repo root, delete folder at code_path.
    $repo_path = str_replace($task->ref->project->drupal_path, '', $task->ref->publish_path);
    if (file_exists($repo_path) ) {
      _provision_recursive_delete($repo_path);
    }
  }

  // When a platform is deleted, if it is the last in the project,
  // and the project has been unpublished, delete the directory.
  if ($task->ref->type == 'platform' && !empty($task->ref->project)) {
    if ($task->ref->project->status == 0 && count($task->ref->project->environments) == 1) {
      // Delete the project folder.
      exec("rm -rf {$project->code_path}");
    }
  }
}

/**
 * Implements hook_post_hosting_TASK_TYPE_task() for install.
 */
function devshop_projects_post_hosting_install_task($task, $data) {
  
  // Queue up a verification of the site.
  hosting_add_task($task->ref->nid, 'verify');
  
}
  
  /**
 * Implements hook_post_hosting_TASK_TYPE_task() for devshop deploy tasks().
 */
function devshop_projects_post_hosting_devshop_deploy_task($task, $data) {

  // Save the deployed git ref to the environment record.
  // Doing this post deploy ensures it was actually checked out.
  drush_log('[DEVSHOP] Environment Deployed.  Saving new git ref to project settings.');

  $git_ref = trim(str_replace('refs/heads/', '', shell_exec("cd {$task->ref->environment->repo_path}; git describe --tags --exact-match || git symbolic-ref -q HEAD")));;

  $args = array(
    ':git' => $git_ref,
    ':site' => $task->ref->environment->site,
  );

  if (db_query('UPDATE {hosting_git} SET git_ref = :git WHERE nid = :site', $args)) {
    drush_log("[DEVSHOP] Git reference {$git_ref} saved to {$task->ref->environment->name} environment record.");
  }
  else {
    return drush_set_error('[DEVSHOP] Environment update failed! Unable to save git ref to hosting_devshop_project_environment table.');
  }

  // Queue up a verification of the platform.
  hosting_add_task($task->ref->environment->platform, 'verify');
  
  // Queue up a verification of the site.
  hosting_add_task($task->ref->nid, 'verify');
}

// @TODO: Remove this function. This is not needed now that we create site nodes at environment create time.
///**
// * Implements hook_post_hosting_TASK_TYPE_task().
// *
// * Acts after the successful verification of platforms.
// * This is where the sites get automatically created.
// */
//function devshop_projects_post_hosting_verify_task($task, $data) {
//
  // If this is a new platform created by a project, we will create a site here.
//  if ($task->ref->type == 'platform') {
//    drush_log('[DevShop] Codebase Clone Complete... Preparing next steps.', 'devshop_log');

//    // If this platform isn't in a project, bail.
//    $platform = $task->ref;
//    if (!isset($platform->project)) {
//      $project = $task->ref->project;
//
//      // @TODO: On first verify, "environment" can be missing. Not sure why, yet.
//      if (empty($platform->environment) && isset($project->environments[$task->project->environment])) {
//        $platform->environment = $project->environments[$task->project->environment];
//      }
//
//      drush_log('[DEVSHOP] No project found for this platform.', 'notice');
//      return;
//    }
//
//    // If the project doesn't have an install profile chosen yet, bail.
//    if (empty($platform->project->install_profile)) {
//      drush_log('[DEVSHOP] No install profile found for this project. Not doing anything.', 'ok');
//      return;
//    }
//
//    // If the platform has a site already, trigger verification, then bail.
//    if (!empty($platform->environment->site)){
//      drush_log('[DEVSHOP] Platform already has a site.', 'notice');
//      // hosting_add_task($platform->environment->site, 'verify');
//      return;
//    }
//    else {
//
//      // Check for site to clone
//      if (!empty($platform->environment->settings->site_to_clone)) {
//        $site = node_load($platform->environment->settings->site_to_clone);
//        $args['target_platform'] = $platform->nid;
//
//        // Ensure new URI doesn't contain underscores or other invalid characters.
//        $args['new_uri'] = str_replace('_', '-', $platform->environment->name) . '.' . $platform->project->base_url;
//
//        // Use the default database server for the project.
//        $args['new_db_server'] = $site->db_server;
//        $args['environment_target'] = $platform->environment->name;
//        drush_log('[DEVSHOP] Cloning environment: ' . $platform->environment->name);
//
//        // Queue the Cloning of the site.
//        // @see devshop_projects_post_hosting_import_task().
//
//        // If, for some reason, we are still missing names, abort.
//        if (empty($site->environment->name) || empty($platform->environment->name)) {
//          return drush_set_error('DEVSHOP_ERROR', 'Source or target environment not found. Contact your systems administrator.');
//        }
//
//        $task = hosting_add_task($platform->environment->settings->site_to_clone, 'clone', $args);
//
//        $link = l(t('View Logs'), "node/{$task->nid}");
//        drush_log("[DevShop] Environment '{$site->environment->name}' is scheduled to be cloned to '{$platform->environment->name}'. $link", 'devshop_log');
//      }
//      else {
//
//        // Determine what DB server to use
//        if (isset($platform->environment->settings->db_server)) {
//          $db_server = $platform->environment->settings->db_server;
//        }
//        else {
//          $db_server = $platform->project->settings->default_environment['db_server'];
//        }
//
//        if (empty($platform->environment->name)) {
//          return drush_set_error('DEVSHOP_ERROR', 'Missing environment name. Unable to create new site.');
//        }
//        else {
//          $new_site = devshop_projects_create_site($platform->project, $platform, $platform->environment->name, $db_server);
//          drush_log("[DevShop] Environment '{$platform->environment->name}' is scheduled to be installed. Please wait...", 'devshop_log');
//        }
//      }
//    }
//  }
//}

///**
// * Implements hook_post_hosting_TASK_TYPE_task().
// */
//function devshop_projects_post_hosting_install_task($task, $data) {
//
//  // Now that environment data is stored in project data we need to verify projects a lot.
//  if ($task->ref->type == 'site' && isset($task->ref->project->nid)) {
//    hosting_add_task($task->ref->project->nid, 'verify');
//  }
//}

// @TODO: Remove this. We no longer use Clone task.
///**
// * Implements hook_post_hosting_TASK_TYPE_task().
// */
//function devshop_projects_post_hosting_clone_task($task, $data) {
//
//  // Now that environment data is stored in project data we need to verify projects a lot.
//  if ($task->ref->type == 'site' && isset($task->ref->project->nid)) {
//    hosting_add_task($task->ref->project->nid, 'verify');
//  }
//}

///**
// * Implements hook_post_hosting_TASK_TYPE_task().
// *
// * Ensures that cloned sites that are on project platforms inherit the project.
// */
//function devshop_projects_post_hosting_import_task($task, $data) {
//
//  // Now that environment data is stored in project data we need to verify projects a lot.
//  if ($task->ref->type == 'site' && isset($task->ref->project->nid)) {
//    hosting_add_task($task->ref->project->nid, 'verify');
//  }
//
//  // If this is a new platform created by a project, we will create a site here.
//  if ($task->ref->type == 'site') {
//    $site = $task->ref;
//    $platform = node_load($site->platform);
//
//    if (isset($platform->environment) && !empty($platform->environment->name)) {
//
//      // Save environment record.
//      drush_log('[DevShop Environment] Saving environment data...', 'devshop_command');
//      db_update('hosting_devshop_project_environment')
//        ->fields(array(
//          'site' => $site->nid,
//        ))
//        ->condition('project_nid', $platform->project->nid)
//        ->condition('name', $platform->environment->name)
//        ->execute();
//      drush_log("Saved {$platform->environment->name} in project {$platform->project->name}.", 'devshop_ok');
//
//      // Save aliases if needed.
//      if ($platform->project->settings->live['environment_aliases'] && !empty($platform->project->settings->live['live_domain'])) {
//        $domain =  $platform->environment->name . '.' . $platform->project->settings->live['live_domain'];
//
//        $site->aliases[] = $domain;
//        drush_log('[DevShop Environment] Saving environment data', 'devshop_command');
//        $site->no_verify = TRUE;
//        node_save($site);
//
//      }
//
//      // If we have been cloned from another environment, run a deploy task.
//      if (!empty($platform->environment->settings->clone_source)) {
//
//        // Trigger a verify of the site, so the alias has project information.
//        hosting_add_task($site->nid, 'verify');
//
//        // Trigger a deploy using the project's default hooks.
//        $args = $platform->project->settings->deploy['default_hooks'];
//        $args['git_ref'] = $platform->environment->git_ref;
//
//        // Handle a Pull Request. If there is one, we want to deploy to the PR branch.
//        if (!empty($platform->environment->github_pull_request->pull_request_object->head->ref)) {
//          $args['git_ref'] = $platform->environment->github_pull_request->pull_request_object->head->ref;
//        }
//
//        hosting_add_task($site->nid, 'devshop-deploy', $args);
//
//      } else {
//        // Trigger a verify of the site, so the alias has project information.
//        hosting_add_task($site->nid, 'verify');
//      }
//    }
//  }
//}
//
///**
// * Utility for execute git commands.
// */
//function _devshop_projects_git_execute($commands, $root = '') {
//  // Execute
//  $return = '';
//  if (is_string($commands)) {
//    $commands = array($commands);
//  }
//  foreach ($commands as $command) {
//    if (!empty($command)) {
//      drush_log("[DEVSHOP] Running Command in $root: $command");
//
//      // @TODO: Create a d()->server->shell_cd_and_exec() function
//      //  server->shell_exec() uses escapeshellcmd(), so we cannot cd_and_exec!
//      // So instead, this is the code from d()->server->shell_exec
//      // d()->server->shell_exec($cmd);
//      if (provision_is_local_host(d()->server->remote_host)) {
//
//        // Run in $root if it exists.
//        if (file_exists($root)) {
//          drush_shell_cd_and_exec($root, escapeshellcmd($command));
//        }
//        else {
//          drush_shell_exec(escapeshellcmd($command));
//        }
//      }
//      else {
//        drush_shell_cd_and_exec($root, 'ssh ' . drush_get_option('ssh-options', '-o PasswordAuthentication=no') . ' %s %s', $this->script_user . '@' . d()->server->remote_host, escapeshellcmd($command));
//      }
//
//      $output = drush_shell_exec_output();
//      drush_log('Shell Output: ' . implode("\n", $output), 'ok');
//      $return .=  implode("\n", $output);
//    }
//  }
//  return $return;
//}
