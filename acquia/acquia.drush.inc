<?php
/**
 * @file acquia.drush.inc
 *
 * Provides DevShop integration with Acquia's Cloud Hooks.
 */

/**
 * Helper for running acquia cloud hooks.
 * @param $hook
 * @param $environment
 */
function drush_run_acquia_hook($hook, $environment) {

  // Collect cloud hook scripts to run for all environments "common".
  $files = scandir("{$environment->repo_root}/hooks/common/{$hook}");
  if (empty($files)) $files = array();

  // The list of files we will run.
  $scripts = array();

  // Remove incorrect entries from scandir.
  $files = array_diff($files, array('..', '.', '.gitignore'));

  // Collect the full path to each script.
  foreach ($files as $script) {
    $scripts[] = realpath("{$environment->repo_root}/hooks/common/{$hook}/{$script}");
  }

  // Collect cloud hook scripts to run for this environment.
  $files = scandir("{$environment->repo_root}/hooks/{$environment->name}/{$hook}");
  if (!empty($files)) {

    // Remove incorrect entries from scandir.
    $files= array_diff($files, array('..', '.', '.gitignore'));

    // Collect the full path to each script.
    foreach ($files as $script) {
      $scripts[] = realpath("{$environment->repo_root}/hooks/{$environment->name}/{$hook}/{$script}");
    }
  }

  // Run Scripts
  // @TODO: Implement using symfony:Process component.
  // Usage: post-code-deploy site target-env source-branch deployed-tag repo-url repo-type
  foreach ($scripts as $file) {
    drush_log('[DEVSHOP] Running Acquia Cloud Hook: ' . $file, 'ok');

    // This is a trick.  DevShop doesn't have aliases like $project.$environment.
    // When Acquia cloud hook writers create a script, they will use something like:
    //
    //    drush_alias=$site'.'$target_env
    //    drush @$drush_alias cc all
    //
    // So, by passing "environment->name" as $site, and project.hostname as $target_env,
    // we can trick the Acquia cloud script to use the right alias.
    if (drush_shell_exec("sh $file {$environment->name} {$environment->project_name}.{$environment->web_server} {$environment->name} old_branch {$environment->git_ref} repo_url repo_type devshop $environment->url") !== 0) {
      $output = drush_shell_exec_output();
      drush_log(implode("\n", $output), 'ok');
    }
    else {
      return drush_set_error(DRUSH_FRAMEWORK_ERROR, 'The last cloud hook returned a non-zero exit code.  Remaining hooks skipped.');
    }
  }
}

/**
 * Implements drush_HOOK_post_COMMAND()
 * for provision_devshop_deploy.
 *
 * Checks for Acquia cloud hooks and runs them.
 */
function drush_acquia_post_provision_devshop_deploy($branch)
{

  if (d()->type == 'site') {
    $environment = (object) d()->environment;

    // Respect drush option, but default to environment settings.
    if (!drush_get_option('acquia_hooks', $environment->settings->deploy['acquia_hooks'])) {
      drush_log('[.hooks] Environment not configured to run acquia cloud hook commands.', 'info');
      return;
    }

    $project_name = d()->project;
    $project = (object) d("@project_{$project_name}")->project;

    // If project has no path to drupal, we know it's not acquia.
    if ($project->drupal_path != 'docroot' || !file_exists(
        $environment->repo_root.'/hooks'
      )
    ) {
      drush_log(
        '[DEVSHOP] ./docroot or ./hooks folder is missing. Not an acquia repo!',
        'ok'
      );

      return;
    }

    // If "manual" deployment, it's a post-code-deploy
    if (drush_get_option('manual', FALSE)) {
      $hook = 'post-code-deploy';
    }
    // If it is automatic, it's a "post-code-update"
    else {
      $hook = 'post-code-update';
    }

    drush_log("[DEVSHOP] Invoking acquia hooks for hook $hook in environment {$environment->name}", 'ok');

    drush_run_acquia_hook($hook, $environment);
  }
}
