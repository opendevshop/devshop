<?php

use Asm\Ansible\Ansible;


/**
 * @file
 * Provides the Ansible Apache service driver.
 */

class Provision_Service_http_ansible_haproxy extends Provision_Service_http_apache_ssl {

    /**
     * @var Ansible;
     */
    private $inventory;
    private $ansible_config_file;
    private $ansible_config;

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
        drush_log('Platform Verified', 'p_log');
    }

    function verify_site_cmd() {
        parent::verify_site_cmd();
        drush_log('Site Verified', 'p_log');
    }


    /**
     * Return the inventory file from ansible configuration.
     *
     * @return string
     */
    protected function getAnsibleInventory() {
        if (!$this->inventory) {
            $this->ansible_config = $this->getAnsibleConfig();
            if (isset($this->ansible_config['inventory'])) {
                $this->inventory = $this->ansible_config['inventory'];
            }
            else {
                $this->inventory = '/etc/ansible/hosts';
            }
        }
        return $this->inventory;
    }

    /**
     * Loads ansible configuration from the default ansible.cfg files.
     *
     * @see http://docs.ansible.com/ansible/intro_configuration.html
     *
     * @return array
     */
    protected function getAnsibleConfig() {
        $ansible_cfg[] = getenv('ANSIBLE_CONFIG');
        $ansible_cfg[] = getcwd() . '/ansible.cfg';
        $ansible_cfg[] = getenv('HOME') . '/.ansible.cfg';
        $ansible_cfg[] = '/etc/ansible/ansible.cfg';

        foreach ($ansible_cfg as $path) {
            if (file_exists($path)) {
                $this->ansible_config_file = $path;
                $config = @parse_ini_file($this->ansible_config_file);
                if (is_array($config)) {
                    return $config;
                }
                else {
                    return array();
                }
            }
        }
        return array();
    }

    function getHostname() {
        return $this->server->remote_host;
    }
}
