<?php
/**
 * @file acquia.drush.inc
 *
 * Provides DevShop integration with Acquia's Cloud Hooks.
 */

/**
 * Implements drush_HOOK_post_COMMAND()
 * for provision_devshop_deploy.
 *
 * Checks for Acquia cloud hooks and runs them.
 */
function drush_acquia_post_provision_devshop_deploy($branch) {

  if (d()->type == 'site'){
    $environment = (object) d()->environment;

    // Respect environment settings.
    if ($environment->settings['deploy']['acquia_hooks'] != 1) {
      return;
    }

    $project_name = d()->project;
    $project = (object) d("@project_{$project_name}")->project;

    // If project has no path to drupal, we know it's not acquia.
    if ($project->drupal_path != 'docroot') {
      drush_log('[DEVSHOP] Acquia Cloud skipped.', 'ok');
      return;
    }

    // Detect "update" vs "deploy"
    $cloud_hooks_path = d()->environment->repo_root . "/hooks";

    // Detect hook files.
    if (file_exists($cloud_hooks_path)) {
      drush_log('[DEVSHOP] Acquia Cloud Hooks not detected...', 'ok');
    }

    // If "manual" deployment, it's a post-code-deploy
    if (drush_get_option('manual', FALSE)) {
      $hook = 'post-code-deploy';
    }
    // If it is automatic, it's a "post-code-update"
    else {
      $hook = 'post-code-update';
    }

    drush_log("[DEVSHOP] Acquia $hook triggered...", 'ok');

    // Collect scripts to run.
    // Common scripts.
    $files = scandir("{$cloud_hooks_path}/common/{$hook}");
    if (empty($files)) $files = array();
    $scripts = array_diff($files, array('..', '.'));
    foreach ($scripts as &$script) {
      $script = realpath("{$cloud_hooks_path}/common/{$hook}/{$script}");
    }

    // Environment scripts specific scripts
    if (file_exists("{$cloud_hooks_path}/{$environment->name}/{$hook}")) {
      $files = scandir("{$cloud_hooks_path}/{$environment->name}/{$hook}");
      if (empty($files)) $files = array();
      $scripts += array_diff($files, array('..', '.'));
      foreach ($scripts as &$script) {
        $script = realpath("{$cloud_hooks_path}/{$environment->name}/{$hook}/{$script}");
      }
    }

    drush_log(implode("\n", $scripts), 'ok');

    // Run Scripts
    // Usage: post-code-deploy site target-env source-branch deployed-tag repo-url repo-type
    foreach ($scripts as $file) {
      drush_log('[DEVSHOP] Running Acquia Cloud Hook: ' . $file, 'ok');

      if (drush_shell_exec("sh $file {$project_name} {$environment->name} old_branch {$branch} repo_url repo_type devshop $environment->url") !== 0) {
        $output = drush_shell_exec_output();
        drush_log(implode("\n", $output), 'ok');
      }
      else {
        return drush_set_error(DRUSH_FRAMEWORK_ERROR, 'The last cloud hook returned a non-zero exit code.  Remaining hooks skipped.');
      }
    }
  }
}