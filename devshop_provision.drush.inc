<?php

/**
 * @file
 * Provision/Drush hooks for git commands.
 */

/**
 * Implementation of hook_drush_command().
 */
function devshop_provision_drush_command() {
  $items['provision-devshop-revert'] = array(
    'description' => 'Pull from git, verify the site, update the database, revert the site\'s Features, and clear the cache.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
    ),
    'aliases' => array('pdr'),
  );
  $items['provision-devshop-sync'] = array(
    'description' => 'Sync database (and files, coming soon) from a chosen source site.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'revert' => 'Call provision-devshop-revert when done.'
    ),
    'arguments' => array(
      'from' => 'Site alias to sync from.', 
      'to' => 'Site alias to sync to.',
    ), 
    'aliases' => array('pds'),
  );
  return $items;
}