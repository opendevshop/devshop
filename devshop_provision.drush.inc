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
      'sync' => 'sync from the source site, if there is one.'
    ),
    'aliases' => array('pdr'),
  );
  $items['provision-devshop-sync'] = array(
    'description' => 'Sync database and files from a chosen source site.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'source-alias' => 'The alias of the site to sync from.'
    ),
    'aliases' => array('pds'),
  );
  return $items;
}