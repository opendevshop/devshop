<?php

/**
 * @file
 * Provision/Drush hooks for git commands.
 */

/**
 * Implementation of hook_drush_command().
 */
function provision_devshop_deploy_drush_command() {
  $items['provision-devshop-deploy'] = array(
    'description' => 'Pull from git and revert the site\'s Features.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'message' => 'Pull the latest version from the repository and revert to it.'
    ),
    'aliases' => array('pdd'),
  );
  return $items;
}

/**
 * Pre provision-devshop-deploy
 */
function drush_provision_pre_provision_devshop_deploy(){
  provision_git_is_repo();
}


/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_devshop_deploy() {
  $site_path = d()->platform->root;
  $target = d()->name;
  
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO

  // Pull latest version of site
  provision_backend_invoke($target, 'provision-git-pull');

  // Push to remote and do all the other verify tasks
  provision_backend_invoke($target, 'provision-verify');
  
  // Updatedb, in case provision-verify doesn't do it
  provision_backend_invoke($target, 'updb');
  // Revert All Features
  provision_backend_invoke($target, 'features-revert-all');

  // Clear the whole cache, just in case.
  provision_backend_invoke($target, 'cc all');
}

