<?php

/**
 * Implements hook_hosting_feature()
 */
function devshop_pull_hosting_feature() {
  // From hosting_example_hosting_feature().
  $features['pull'] = array(
    'title' => t('DevShop Pull Queue'),
    'description' => t('Provides a queue for pulling a Git powered project.'),
    'status' => HOSTING_FEATURE_ENABLED,
  );
  return $features;
}
