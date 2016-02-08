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
    ),
  );
  return $items;
}

/**
 * Implements the provision-fua command.
 */
function drush_aegir_commit_provision_commit() {

  if (d()->type == 'site') {

    $environment = (object) d()->environment;

    $message = trim(drush_get_option('message', ''));
    $push = drush_get_option('push', FALSE);

  }
}
