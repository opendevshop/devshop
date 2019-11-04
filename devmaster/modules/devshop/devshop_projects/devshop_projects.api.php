<?php
/**
 * @file
 * devshop_projects.api.php
 * Example functions for interacting with devshop.
 */

/**
 * Implements hook_devshop_environment_alter()
 *
 * Activated from devshop_projects_load(), this runs anytime an environment is
 * loaded.
 *
 */
function hook_devshop_environment_alter(&$environment, $project) {
  $environment->tagline = t('Hosted by DevShop');
}

/**
 * Implements hook_devshop_environment_menu()
 *
 * Add items to the Environment Menu ("Hamburger icon")
 *
 * @return array
 */
function hook_devshop_environment_menu($environment) {
  if ($environment->site && $environment->site_status == HOSTING_SITE_ENABLED) {
    $items[] = 'download';
  }
  return $items;
}
