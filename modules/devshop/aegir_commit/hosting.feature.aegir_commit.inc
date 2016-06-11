<?php

/**
 * Implements hook_hosting_feature().
 * @return array List of features to expose to Aegir's hosting features page.
 */
function aegir_commit_hosting_feature() {
  $features['aegir_commit'] = array(
    'title' => t('Commit Code'),
    'description' => t('Commit all or some of the changed files to the git repository.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'aegir_commit',
    'group' => 'advanced',
  );
  return $features;
}
