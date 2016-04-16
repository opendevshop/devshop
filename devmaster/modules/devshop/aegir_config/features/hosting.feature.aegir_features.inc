<?php

/**
 * Implements hook_hosting_feature().
 * @return array List of features to expose to Aegir's hosting features page.
 */
function aegir_features_hosting_feature() {
  $features['aegir_features'] = array(
    'title' => t('Recreate Features'),
    'description' => t('Run "features-update" or "features-update-all" on your site.'),
    'status' => HOSTING_FEATURE_ENABLED,
    'module' => 'aegir_features',
    'group' => 'advanced',
  );
  return $features;
}
