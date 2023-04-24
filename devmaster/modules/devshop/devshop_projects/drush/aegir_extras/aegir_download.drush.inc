<?php

use Symfony\Component\Process\Process;
use GitWrapper\GitWrapper;
use GitWrapper\GitWorkingCopy;
use GitWrapper\GitException;

/**
 * Implementation of hook_drush_command().
 * Provides provision commands for all devshop tasks.
 */
function aegir_download_drush_command()
{
    $items['provision-download'] = array(
        'description' => 'Downloads drupal modules and themes, and optionally commits them to git.',
        'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
        'options' => array(
            'packages' => 'This list of modules and themes to download.',
            'commit' => 'Commit the downloaded code to git.',
            'message' => 'The message to use when committing.',
            'update' => 'Run update.php after the download.',
            'test' => 'Queue a test run after the download.',
        ),
        'arguments' => array(
        ),
        'examples' => array(
            'drush @env.project.domain.com provision-download views ctools --update --commit --message="the usuals"' => 'Downloads views & ctools, commits them to the repo with a message, and run drush updb.',
        ),
        'aliases' => array('pdl'),
    );
    return $items;
}

/**
 * Implements the provision-download command.
 */
function drush_aegir_download_provision_download() {
    $packages = drush_get_option('packages');
    $composer_build = false;

    // Use composer require if there is a composer.json file and packages.drupal.org is a repo...
    if (file_exists(d()->platform->repo_path . '/composer.json')) {
      $composer_json = json_decode(file_get_contents(d()->platform->repo_path . '/composer.json'));
      $cmd = '';

      if (is_string($packages)) {
        $packages = explode(' ', $packages);
      }

      if (isset($composer_json->repositories)) {
        foreach ($composer_json->repositories as $repo) {
          if (strpos($repo->url, 'https://packages.drupal.org') === 0) {
            $cmd = "composer require ";
            foreach ($packages as $package) {
              $cmd .= "drupal/$package ";
              $composer_build = true;
            }
            continue;
          }
        }
      }

      if (empty($cmd)) {
        drush_log(dt('The Drupal packagist repository was not found: unable to use `composer require` to add modules. Please see https://www.drupal.org/docs/develop/using-composer/using-composer-to-manage-drupal-site-dependencies#drupal-packagist'), 'warning');
      }
    }
    else {

      if (is_array($packages)) {
        $packages = implode($packages);
      }

      // No modules, no more.
      if (empty($packages)) {
        return drush_set_error('No packages defined.');
      }

      // Run `drush dl $modules`
      $target = d()->name;
      $cmd = "drush $target dl $packages --yes --strict=0";

      // Pass through options to the drush dl command.
      if (drush_get_option('commit', FALSE)) {
        $cmd .= ' --commit ';
      }
      if (drush_get_option('message', '')) {
        $message = escapeshellarg(drush_get_option('message', dt('Committed by DevShop: ') . $packages));
        $cmd .= " --message='$message' ";
      }
      if (drush_get_option('update', FALSE)) {
        $cmd .= " --update";
      }
    }

    provision_process($cmd, d()->platform->repo_path);

    // Commit and push has to happen here when using composer, there are no post-composer hooks here.
    if ($composer_build && drush_get_option('commit', FALSE)) {
      $message = escapeshellarg(drush_get_option('message', dt('Committed by DevShop: ') . implode(" ", $packages)));

      $files = 'composer.json';
      if (file_exists(d()->platform->repo_path . '/composer.lock')) {
        $files .= ' composer.lock';
      }

      $cmd = "git commit $files --message=$message";
      provision_process($cmd, d()->platform->repo_path);

      $process = drush_get_context('provision_process_result');
      if ($process->getExitCode() == 0) {
        provision_process('git push', d()->platform->repo_path);
      }
    }
}

/**
 * Implements hook_drush_pm_post_download()
 *
 * Runs after a project has been downloaded.
 *
 * This is needed for devshop because we want to commit the new module we just
 * downloaded, and only this hook knows about the path.
 *
 * @param $drupal_project
 * @param $release
 */
function aegir_download_drush_pm_post_download($drupal_project, $release) {

  $project_name = d()->project;
  $project = (object) d("@project_{$project_name}")->project;
  $environment = (object) $project->environments[d()->hosting_environment];

  if (d()->hosting_environment && drush_get_option('commit', FALSE)) {
    drush_log(dt('[DEVSHOP] Committing new module...'), 'ok');

    $message = drush_get_option('message', dt('Committed by DevShop: ') . $drupal_project['name']);

    $wrapper = new GitWrapper();
    $wrapper->streamOutput();

    try {
      $git = $wrapper->workingCopy($environment->repo_path);

      $download_path = $drupal_project['full_project_path'];
      $git
        ->add($download_path)
        ->commit($message)
        ->push();
    }
    catch (GitException $e) {
      return drush_set_error('DRUSH_ERROR', "Git Exception: " . $e->getMessage());
    }
  }
  else {
    drush_log(dt('Skipping Commit. Make sure you commit or delete this module at some point.'), 'notice');
  }
}
