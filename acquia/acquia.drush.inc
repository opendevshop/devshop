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

  // Collect scripts to run.
  // Common scripts.
  $files = scandir("{$environment->repo_root}/hooks/common/{$hook}");
  if (empty($files)) $files = array();
  $scripts = array_diff($files, array('..', '.'));
  foreach ($scripts as &$script) {
    $script = realpath("{$environment->repo_root}/hooks/common/{$hook}/{$script}");
  }

  // Environment scripts: Post Code Update
  if (file_exists("{$environment->repo_root}/hooks/{$environment->name}")) {
    $files = scandir("{$environment->repo_root}/hooks/{$environment->name}/{$hook}");
    if (empty($files)) $files = array();
    $scripts += array_diff($files, array('..', '.'));
    foreach ($scripts as &$script) {
      $script = realpath("{$environment->repo_root}/hooks/{$environment->name}/{$hook}/{$script}");
    }
  }

  // Run Scripts
  // @TODO: Implement using symfony:Process component.
  // Usage: post-code-deploy site target-env source-branch deployed-tag repo-url repo-type
  foreach ($scripts as $file) {
    drush_log('[DEVSHOP] Running Acquia Cloud Hook: ' . $file, 'ok');

    if (drush_shell_exec("sh $file {$environment->project_name} {$environment->name} old_branch {$environment->git_ref} repo_url repo_type devshop $environment->url") !== 0) {
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

    // Respect environment settings.
    if ($environment->settings['deploy']['acquia_hooks'] != 1) {
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

    drush_run_acquia_hook('post-code-deploy', $environment);
  }
}
