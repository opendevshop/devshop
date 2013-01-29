<?php

/**
 * Pre provision-devshop-delete
 */
function drush_devshop_provision_pre_provision_devshop_delete() {
  $args = func_get_args();

  $sites = explode(' ', $args[2]);

  if (!empty($sites)) {
    foreach ($sites as $key => $alias) {
      drush_invoke('provision-delete', $alias);
      drush_log(dt('Deleted @site', array('@site' => $alias)));
    }
  }
}


/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_delete() {
  $args = func_get_args();

  $platforms = explode(' ', $args[1]);
  $data = explode(' ', $args[0]);

  drush_log(dt('Provision DevShop Delete started...'));
  
  drush_log(dt("Deleting platforms"));

  foreach ($platforms as $key => $alias) {
    drush_invoke('provision-delete', $alias);
    drush_log(dt('Deleted @platform', array('@platform' => $alias)));
  }
}

