<?php

/**
 * Pre provision-git-features-update-commit hook
 */
function drush_provision_pre_provision_devshop_dl(){
  provision_git_is_repo();
}

/**
 * Implements the provision-git-pull command.
 */
function drush_devshop_provision_provision_devshop_dl() {
  $modules = drush_get_option('modules');
  $target = d()->name;
  provision_backend_invoke($target, "dl $modules");
}