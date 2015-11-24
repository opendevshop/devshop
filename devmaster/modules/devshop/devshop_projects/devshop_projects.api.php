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
function hook_devshop_environment_alter(&$environment) {
  $environment->tagline = t('Hosted by DevShop');
}