<?php
/**
 * @file
 * Provides the MySQL service driver.
 */

class Provision_Service_http_ansible_apache extends Provision_Service_http {

    /**
     * Override sync() method since we don't need to sync.
     */
    public function sync() {
    }
}
