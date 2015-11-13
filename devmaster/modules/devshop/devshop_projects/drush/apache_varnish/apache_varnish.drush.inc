<?php

/**
 * Implements hook_drush_init().
 */
function apache_varnish_drush_init() {
    apache_varnish_provision_register_autoload();
}

/**
 * Register our directory as a place to find provision classes.
 */
function apache_varnish_provision_register_autoload() {
    static $loaded = FALSE;
    if (!$loaded) {
        $loaded = TRUE;
        provision_autoload_register_prefix('Provision_', dirname(__FILE__));
    }
}

/**
 * Implements hook_provision_services().
 */
function apache_varnish_provision_services() {
    apache_varnish_provision_register_autoload();
    return array('http_apache_varnish' => NULL);
}
