<?php
/**
 * @file
 *   Expose the extra tasks feature to hostmaster.
 */

/**
 * Implements hook_hosting_feature().
 */
function ansible_playbooks_hosting_feature() {
  $features['playbooks'] = array(
    'title' => t('Ansible Playbooks'),
    'description' => t("Run Ansible playbooks with Aegir."),
    'status' => HOSTING_FEATURE_ENABLED,
    'module' => 'ansible_playbooks',
    'group' => 'advanced',
  );
  return $features;
}

