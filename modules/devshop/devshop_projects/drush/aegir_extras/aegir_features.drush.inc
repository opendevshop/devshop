<?php

use Symfony\Component\Process\Process;
use GitWrapper\GitWrapper;
use GitWrapper\GitWorkingCopy;
use GitWrapper\GitException;

/**
 * Implementation of hook_drush_command().
 * Provides provision commands for all devshop tasks.
 */
function aegir_features_drush_command()
{
  $items['provision-features_update'] = array(
    'description' => 'Runs drush features-update-all.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'features' => 'The list of features to update.',
      'revert' => 'Whether or not to revert the features afterwards.',
    ),
  );
  return $items;
}

/**
 * Implements the provision-fua command.
 */
function drush_aegir_features_provision_features_update() {

  if (d()->type == 'site') {

    $project_name = d()->project;
    $project = (object) d("@project_{$project_name}")->project;
    $environment = (object) $project->environments[d()->environment];

    $features = trim(drush_get_option('features', ''));
    $revert = drush_get_option('revert', FALSE);

    // If features were specified...
    if (!empty($features)) {
      provision_process("drush {$environment->drush_alias} features-update {$features} -y", NULL, dt('Update Features'));
    }
    else {
      provision_process("drush {$environment->drush_alias} features-update-all -y", NULL, dt('Update All Features'));
    }

    // Show git status
    provision_process("git status", $environment->repo_path, NULL, dt('Checking git status'));

    // Revert force revert the features if the user asked.
    if ($revert) {
      if (!empty($features)) {
        provision_process("drush {$environment->drush_alias} fr {$features} --force -y", NULL, dt('Revert Features'));
      }
      else {
        provision_process("drush {$environment->drush_alias} fra --force -y", NULL, dt('Revert Features'));
      }
    }
  }
}
