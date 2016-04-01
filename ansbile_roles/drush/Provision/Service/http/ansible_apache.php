<?php
/**
 * @file
 * Provides the MySQL service driver.
 */

class Provision_Service_http_ansible_apache extends Provision_Service_http {

    /**
     * Override sync() method because we don't need to sync.
     */
    function sync($path = NULL, $additional_options = array()) {
//        return $this->server->sync($path, $additional_options);
    }

    function verify_server_cmd() {
        drush_log('Server Verified', 'devshop_log');
    }

    function verify_platform_cmd() {
        drush_log('Platform Verified', 'devshop_log');
    }

    function verify_site_cmd() {
        drush_log('Site Verified', 'devshop_log');
    }
}
