<?php
/**
 * @file
 * Provide services for MySQL and Apache via Ansible.
 */

/**
 * Implements hook_hosting_feature().
 */
function ansible_services_hosting_feature() {
    $features['ansible_services'] = array(
        'title' => t('Ansible Services'),
        'description' => t('Configure Apache and MySQL with Ansible.'),
        'status' => HOSTING_FEATURE_DISABLED,
        'module' => 'ansible_services',
        'group' => 'advanced',
    );
    return $features;
}
