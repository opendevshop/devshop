<?php
/**
 * @file
 * Provides the Ansible MySQL service driver.
 */

class Provision_Service_db_ansible_mysql extends Provision_Service_db_mysql {

    function verify_server_cmd() {
        parent::verify_server_cmd();
        drush_log('Server Verified', 'p_log');
    }

    function verify_platform_cmd() {
        parent::verify_platform_cmd();
        drush_log('Platform Verified', 'p_log');
    }

    function verify_site_cmd() {
        parent::verify_site_cmd();
        drush_log('Site Verified', 'p_log');
    }
}
