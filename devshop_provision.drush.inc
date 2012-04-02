<?php

/**
 * @file
 * Provision/Drush hooks for git commands.
 */

/**
 * Implementation of hook_drush_command().
 */
function devshop_provision_drush_command() {
  $items['provision-pull'] = array(
    'description' => 'Pull & verify site code and (optionally) run update.php, clear cache, and revert features.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'sync' => 'Sync content from default source site before code pull.',
      'update' => 'Run update.php after code pull.',
      'revert' => 'Revert all features after code pull.',
      'cache' => 'Clear all caches after code pull.',
    ),
    'aliases' => array('pp'),
  );
  $items['provision-sync'] = array(
    'description' => 'Sync database (and files, coming soon) from a chosen source site.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'update' => 'Run update.php after content sync.',
      'revert' => 'Revert all features after content sync.',
      'cache' => 'Clear all caches after content sync.',
    ),
    'arguments' => array(
      'from' => 'Site alias to sync from.', 
      'to' => 'Site alias to sync to.',
    ), 
    'aliases' => array('ps'),
  );
  $items['provision-commit'] = array(
    'description' => 'Export the site\'s Features and commit the result.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'revert' => ' Force revert all features after exporting and committing.',
    ),
    'aliases' => array('pc'),
    'arguments' => array(
      'from' => 'Site alias to sync from.',
    ),
  );
  return $items;
}