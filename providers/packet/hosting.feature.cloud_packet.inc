<?php
/**
 * @file
 * Register this module as a Hosting feature.
 */

/**
 * Implements hook_hosting_feature().
 */
function aegir_cloud_packet_hosting_feature() {
  $features['cloud_packet'] = array(
    'title' => t('Packet.net Cloud Servers'),
    'description' => t('Create new servers using Packet.net.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'aegir_cloud_packet',
    'group' => 'experimental',
  );
  return $features;
}
