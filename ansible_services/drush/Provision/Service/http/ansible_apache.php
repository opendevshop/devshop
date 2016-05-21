<?php

require_once(__DIR__ . '/../../../../../vendor/autoload.php');

use Asm\Ansible\Ansible;


/**
 * @file
 * Provides the Ansible Apache service driver.
 */

class Provision_Service_http_ansible_apache extends Provision_Service_http_apache_ssl {

    function init_platform() {
        drush_log('Provision_Service_http_ansible_apache::init_platform', 'status');

    }

    function init_site() {
        drush_log('Provision_Service_http_ansible_apache::init_site', 'status');

    }

    function verify_server_cmd() {
    }

    function verify_platform_cmd() {
        parent::verify_platform_cmd();
        drush_log('Platform Verified', 'devshop_log');
    }

    function verify_site_cmd() {
        parent::verify_site_cmd();
        drush_log('Site Verified', 'devshop_log');
    }
}
