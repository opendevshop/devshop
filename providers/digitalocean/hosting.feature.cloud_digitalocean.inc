<?php
/**
 * @file
 * Register this module as a Hosting feature.
 */

/**
 * Implements hook_hosting_feature().
 */
function aegir_digitalocean_hosting_feature() {
  $features['cloud_digitalocean'] = array(
    'title' => t('DigitalOcean Cloud Servers'),
    'description' => t('Create new servers using DigitalOcean.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'aegir_digitalocean',
    'group' => 'experimental',
  );
  return $features;
}
