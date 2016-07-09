<?php

/**
 * Implements hook_hosting_feature().
 * @return array List of features to expose to Aegir's hosting features page.
 */
function aegir_update_hosting_feature() {
  $features['aegir_update'] = array(
    'title' => t('Update Drupal'),
    'description' => t('Upgrades drupal core and contrib to the latest versions.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'aegir_update',
    'group' => 'advanced',
  );
  return $features;
}
