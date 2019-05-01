<?php
/**
 * @file
 */

/**
 * Implements hook_hosting_feature().
 */
function ansible_core_hosting_feature() {
    $features['ansible_core'] = array(
        'title' => t('Aegir Ansible Core'),
        'description' => t('Base module for other Ansible features.'),
        'status' => HOSTING_FEATURE_DISABLED,
        'module' => 'aegir_ansible_core',
        'group' => 'advanced',
    );
    return $features;
}
