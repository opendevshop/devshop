<?php
/**
 * @file
 * Register this module as a Hosting feature.
 */

/**
 * Implements hook_hosting_feature().
 */
function aegir_softlayer_hosting_feature() {
  $features['cloud_softlayer'] = array(
    'title' => t('Softlayer Cloud Servers'),
    'description' => t('Create new servers using Softlayer.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'aegir_softlayer',
    'group' => 'experimental',
  );
  return $features;
}
