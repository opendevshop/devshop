<?php

/**
 * Implements hook_hosting_feature().
 * @return array List of features to expose to Aegir's hosting features page.
 */
function devshop_hosting_hosting_feature() {
  $features['devshop_projects'] = array(
    'title' => t('DevShop'),
    'description' => t('DevShop Hosting Platform'),
    'status' => HOSTING_FEATURE_REQUIRED,
    'module' => 'devshop_projects',
    'group' => 'advanced',
  );
  $features['devshop_testing'] = array(
    'title' => t('DevShop Testing'),
    'description' => t('Allows running of tests on sites.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_testing',
    'group' => 'advanced',
  );
  $features['devshop_dothooks'] = array(
    'title' => t('DevShop .Hooks'),
    'description' => t('Allows projects to define their deploy hooks in a .hooks.yml file.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_dothooks',
    'group' => 'advanced',
  );
  $features['devshop_acquia'] = array(
    'title' => t('DevShop Acquia'),
    'description' => t('Allows environments to use Acquia Cloud Hooks for Deployment Hooks.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_acquia',
    'group' => 'advanced',
  );
  $features['devshop_remotes'] = array(
    'title' => t('DevShop Remote Aliases'),
    'description' => t('Allows projects to add remote drush aliases as data sources.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'devshop_remotes',
    'group' => 'advanced',
  );
  return $features;
}
