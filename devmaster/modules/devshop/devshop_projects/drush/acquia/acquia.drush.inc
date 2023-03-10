<?php
/**
 * @file acquia.drush.inc
 *
 * Provides DevShop integration with Acquia's Cloud Hooks.
 */

/**
 * Implements drush_HOOK_post_COMMAND()
 * for provision_deploy.
 *
 * Checks for Acquia cloud hooks and runs them.
 */
function drush_acquia_post_provision_deploy($branch = '') {

  if (d()->type == 'site' && d()->environment_settings && d()->project){

    if (empty($branch)) {
      $branch = d()->platform->git_ref;
    }

    $project_name = d()->project;
    $project = (object) d("@project_{$project_name}")->project;
    $environment = (object) $project->environments[d()->environment];

    // Respect environment settings.
    if ($environment->settings['deploy']['acquia_hooks'] != 1) {
      return;
    }

    // If project has no path to drupal, we know it's not acquia.
    if ($project->drupal_path != 'docroot') {
      drush_log('[DEVSHOP] Acquia Cloud skipped.', 'ok');
      return;
    }

    // Detect hook files.
    $cloud_hooks_path = $environment->repo_path . '/hooks';
    if (file_exists($cloud_hooks_path)) {
      drush_log('[DEVSHOP] Acquia Cloud Hooks detected...', 'ok');
    }

    // Collect scripts to run.
    // Common scripts.
    $files = scandir($cloud_hooks_path . '/common/post-code-update');
    if (empty($files)) $files = array();
    $scripts = array_diff($files, array('..', '.'));
    foreach ($scripts as &$script) {
      $script = realpath($cloud_hooks_path . '/common/post-code-update/' . $script);
    }

    // Environment scripts: Post Code Update
    if (file_exists($cloud_hooks_path . '/' . $environment->name)) {
      $files = scandir($cloud_hooks_path . '/' . $environment->name . '/post-code-update');
      if (empty($files)) $files = array();
      $scripts += array_diff($files, array('..', '.'));
      foreach ($scripts as &$script) {
        $script = realpath($cloud_hooks_path . '/' . $environment->name . '/post-code-update/' . $script);
      }
    }

    // @TODO: Post Code Deploy & Post DB Copy

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