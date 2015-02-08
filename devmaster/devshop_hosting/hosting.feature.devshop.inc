<?php

/**
 * Implements hook_hosting_feature
 * @return array List of features to expose to Aegir's hosting features page.
 */
function devshop_hosting_hosting_feature() {
  $features['devshop_hosting'] = array(
    'title' => t('DevShop'),
    'description' => t('Enables the project-centric DevShop UI.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_hosting',
    'group' => 'devshop',
  );
  $features['devshop_projects'] = array(
    'title' => t('DevShop Projects'),
    'description' => t('Enables the project-centric DevShop UI.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_projects',
    'group' => 'devshop',
  );
  $features['devshop_pull'] = array(
    'title' => t('DevShop Pull'),
    'description' => t('Allows automated Pull Code on commit or via queue.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_projects',
    'group' => 'devshop',
  );
  $features['devshop_testing'] = array(
    'title' => t('DevShop Testing'),
    'description' => t('Allows running of tests on sites.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_testing',
    'group' => 'devshop',
  );
  $features['devshop_log'] = array(
    'title' => t('DevShop Logs'),
    'description' => t('Provides error and commit logs to DevShop.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_log',
    'group' => 'devshop',
  );
  return $features;
}
