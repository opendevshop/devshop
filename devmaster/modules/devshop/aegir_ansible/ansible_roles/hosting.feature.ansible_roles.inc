<?php
/**
 * @file
 * Expose the db_server feature to hostmaster.
 */

/**
 * Implements hook_hosting_feature().
 */
function ansible_roles_hosting_feature() {
  $features['ansible_roles'] = array(
    'title' => t('Custom Ansible Roles'),
    'description' => t('Configure servers using Ansible roles. WARNING: This module allows downloading any Ansible Galaxy role through the Drupal Web UI.'),
    'status' => HOSTING_FEATURE_DISABLED,
    'module' => 'ansible_roles',
    'group' => 'experimental',
  );
  return $features;
}
