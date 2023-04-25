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
function drush_devshop_projects_pre_hosting_task()
{

  $task =& drush_get_context('HOSTING_TASK');

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
  if ($task->ref->type == 'site' && $task->task_type == 'devshop-deploy' && !empty($task->args['git_reference'])) {

    // The git_ref is the only argument
    $task->args['git_ref'] = $task->task_args['git_ref'];

    // Just load in all of the task_args as options(they come from the task form.)
    if (is_array($task->task_args)) {
      foreach ($task->task_args as $arg => $value) {
        $task->options[$arg] = $value;
      }
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
  }
}

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
    $task->ref->environments = NULL;
    node_save($task->ref);
  }

  // When a site is deleted (if it is in a project, delete the platform it is on.
  // @TODO: Check for another site on this platform?
  if ($task->ref->type == 'site' && !empty($task->ref->project)) {
    // We trigger platform deletion here.
    hosting_add_task($task->ref->platform, 'delete');
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

  // @TODO: remove? refactor?
//  if (db_query('UPDATE {hosting_git} SET git_ref = :git WHERE nid = :site', $args)) {
//    drush_log("[DEVSHOP] Git reference {$git_ref} saved to {$task->ref->environment->name} environment record.");
//  }
//  else {
//    return drush_set_error('[DEVSHOP] Environment update failed! Unable to save git ref to hosting_devshop_project_environment table.');
//  }

  // Queue up a verification of the platform.
  // @TODO: Find a way to pass github_deployment to this task too.
  hosting_add_task($task->ref->environment->platform, 'verify');

  // Queue up a verification of the site.
  // @TODO: Find a way to pass github_deployment to this task too.
  hosting_add_task($task->ref->nid, 'verify');
}
