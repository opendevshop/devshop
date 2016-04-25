<?php

use Symfony\Component\Process\Process;
use GitWrapper\GitWrapper;
use GitWrapper\GitWorkingCopy;
use GitWrapper\GitException;

/**
 * Implementation of hook_drush_command().
 * Provides provision commands for all devshop tasks.
 */
function aegir_commit_drush_command()
{
  $items['provision-commit'] = array(
    'description' => 'Commits code to git.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'message' => 'The commit message to use.',
      'push' => 'Push the code after committing.',
      'files' => 'A list of files, if desired.',
      'name' => 'The name of the user to commit as.',
      'mail' => 'The email of the user to commit as.',
    ),
  );
  return $items;
}

/**
 * Implements the provision-fua command.
 */
function drush_aegir_commit_provision_commit() {

  if (d()->type == 'site') {

    $project_name = d()->project;
    $project = (object) d("@project_{$project_name}")->project;
    $environment = (object) $project->environments[d()->environment];

    $git_url = $project->git_url;

    $message = trim(drush_get_option('message', ''));
    $files = json_decode(drush_get_option('files', '[]'));
    $push = drush_get_option('push', FALSE);
    $name = drush_get_option('name', FALSE);
    $mail = drush_get_option('mail', FALSE);

    // Git Add
    try {
      $wrapper = new GitWrapper();
      $wrapper->streamOutput();

      $git = $wrapper->workingCopy($environment->repo_root);
      $status = $git->getStatus();

      if (empty($status)) {
        drush_log('[Git Status] Working Directory is clean. Nothing to commit.', 'devshop_log');
      }
      else {
        devshop_drush_process('git status', $environment->repo_root, dt('Git Status'));

        // If files option was detected
        if ($files) {
          $files_list = implode(' ', $files);
          $cmd = "git add {$files_list}";
        }
        else {
          $cmd = 'git add -A';
        }

        devshop_drush_process($cmd, $environment->repo_root, 'Adding files to local repository...');
        devshop_drush_process('git status', $environment->repo_root, dt('Git Status'));

        $hostmaster_uri = d('hostmaster')->uri;
        $message = escapeshellarg("$message
--------------
Committed by OpenDevShop via http://$hostmaster_uri");

        devshop_drush_process("git commit -m {$message}", $environment->repo_root, 'Committing changes to local repository...', array(
          'GIT_AUTHOR_NAME' => $name,
          'GIT_AUTHOR_EMAIL' => $mail,
          'GIT_COMMITTER_NAME' => $name,
          'GIT_COMMITTER_EMAIL' => $mail,
        ));

        devshop_drush_process('git status', $environment->repo_root, dt('Git Status'));
      }

      // Push the result, if desired.
      if ($push) {

        // Do a little pre-check to ensure we don't hang at http password.
        $git_url = parse_url($git_url);
        if (($git_url['scheme'] == 'http' || $git_url['scheme'] == 'https') && empty($git_url['pass'])) {
          drush_log('http git url detected, with no password. Unable to push to this repository.', 'warning');
          return;
        }
        else {
          devshop_drush_process("git push", $environment->repo_root, 'Pushing code to remote repository...');
          devshop_drush_process('git status', $environment->repo_root, dt('Git Status'));
        }
      }
    }
    catch (GitException $e) {
      drush_log('Git Error', 'devshop_command');
      drush_log("Unable to load git repository at path '{$environment->repo_root}'", 'devshop_error');
      drush_set_error($e->getMessage());
    }
  }
}
