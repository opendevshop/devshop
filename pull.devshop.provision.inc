<?php



/**
 * Pre provision-devshop-deploy
 */
function drush_provision_pre_provision_devshop_revert(){
  provision_git_is_repo();
}


/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_revert() {
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

