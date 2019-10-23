<?php

/**
 * Implements hook_hosting_feature().
 * @return array List of features to expose to Aegir's hosting features page.
 */
function aegir_download_hosting_feature() {
  $features['aegir_download'] = array(
    'title' => t('Download Modules'),
    'description' => t('Add modules or themes to your git repository.'),
    'status' => HOSTING_FEATURE_ENABLED,
    'module' => 'aegir_download',
    'group' => 'advanced',
  );
  return $features;
}
