<?php


/**
 * @file
 * The hosting feature definition for server data example.
 */

/**
 * Register a hosting feature with Aegir.
 *
 * This will be used to generate the 'admin/hosting' page.
 *
 * @return array
 *   associative array indexed by feature key.
 */
function ansible_variables_hosting_feature() {
  $features['ansible_variables'] = array(
    'title' => t('Ansible Variables'),
    'description' => t("Allows editing of a server's ansible variables."),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'ansible_variables',
    'group' => 'advanced',
  );
  return $features;
}
