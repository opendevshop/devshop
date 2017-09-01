<?php

use GitWrapper\GitException;
use GitWrapper\GitWrapper;

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
    $git_url = d()->platform->repo_url;
    $git_path = d()->platform->repo_path;

    $message = trim(drush_get_option('message', ''));
    $files = json_decode(drush_get_option('files', '[]'));
    $push = drush_get_option('push', FALSE);
    $name = drush_get_option('name', FALSE);
    $mail = drush_get_option('mail', FALSE);

    // Git Add
    try {
      $wrapper = new GitWrapper();
      $wrapper->streamOutput();

      $git = $wrapper->workingCopy($git_path);
      $status = $git->getStatus();

      if (empty($status)) {
        drush_log('[Git Status] Working Directory is clean. Nothing to commit.', 'devshop_log');
      }
      else {
        devshop_drush_process('git status', $git_path, dt('Git Status'));
        $cmds = array();

        // If files option was detected
        if ($files) {
          $files_list = '';

          foreach ($files as $file) {
            $file = $git_path . '/' . $file;
            if (file_exists($file)) {
              $files_list .= ' ' . $file;
            }
          }

          // If files lists is not empty, add "git add" command to queue.
          if (!empty($files_list)) {
            $cmds[] = "git add {$files_list}";
          }
        }
        else {
          $cmds[] = 'git add -A';
        }

        foreach ($cmds as $cmd) {
          devshop_drush_process($cmd, $git_path, 'Adding files to local repository...');
        }

        devshop_drush_process('git status', $git_path, dt('Git Status'));

        $hostmaster_uri = d('hostmaster')->uri;
        $message = escapeshellarg("$message
--------------
Committed by OpenDevShop via http://$hostmaster_uri");

        devshop_drush_process("git commit -m {$message}", $git_path, 'Committing changes to local repository...', array(
          'GIT_AUTHOR_NAME' => $name,
          'GIT_AUTHOR_EMAIL' => $mail,
          'GIT_COMMITTER_NAME' => $name,
          'GIT_COMMITTER_EMAIL' => $mail,
        ));

        devshop_drush_process('git status', $git_path, dt('Git Status'));
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
          devshop_drush_process("git push", $git_path, 'Pushing code to remote repository...');
          devshop_drush_process('git status', $git_path, dt('Git Status'));
        }
      }
    }
    catch (GitException $e) {
      drush_log('Git Error', 'devshop_command');
      drush_log("Unable to load git repository at path " . $git_path, 'devshop_error');
      drush_set_error($e->getMessage());
    }
  }
}
