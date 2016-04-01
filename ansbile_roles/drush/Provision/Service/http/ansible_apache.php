<?php

require_once(__DIR__ . '/../../../../../vendor/autoload.php');

use Asm\Ansible\Ansible;

/**
 * @file
 * Provides the Ansible Apache service driver.
 */

class Provision_Service_http_ansible_apache extends Provision_Service_http {

    /**
     * @var Ansible;
     */
    private $ansible;
    private $inventory;
    private $playbook;
    private $config_file;
    private $config;

    /**
     * Override sync() method because we don't need to sync.
     */
    function sync($path = NULL, $additional_options = array()) {
        parent::sync($path, $additional_options);
    }

    function verify_server_cmd() {

        // If "inventory" exists in ansible configuration, use that instead of the default '/etc/ansible/hosts'
        if ($this->getAnsibleInventory()) {
            drush_log('Ansible Config Loaded from ' . $this->config_file, 'ok');
            $this->inventory = $this->getAnsibleInventory();
        }

        // Last check: does the inventory file exist?
        if (!file_exists($this->inventory)) {
            throw new \Exception('No file was found at the path specified by the "inventory-file" option: ' . $this->inventory);
        }

        // Look for playbook
        $this->playbook = drush_get_option('playbook', realpath('../../../../../playbook.yml'));

        // Last check: does the playbook file exist?
        if (!file_exists($this->playbook)) {
            throw new \Exception('No file was found at the path specified by the "playbook" option: ' . $this->playbook);
        }

        // Prepare the Ansible object.
        $this->ansible = new Ansible(
            getcwd(),
            '/usr/bin/ansible-playbook',
            '/usr/bin/ansible-galaxy'
        );

        $ansible = $this->ansible->playbook();

        $ansible->play($this->playbook);

        if ($this->ansible_user) {
            $ansible->user($this->ansible_user);
        }
        if ($this->server->title) {
            $ansible->limit($this->server->title);
        }

        if ($this->inventory) {
            $ansible->inventoryFile($this->inventory);
        }

        $ansible->execute(function ($type, $buffer) {
            print $buffer;
        });
    }

    function verify_platform_cmd() {
        parent::verify_platform_cmd();
        drush_log('Platform Verified', 'devshop_log');
    }

    function verify_site_cmd() {
        parent::verify_site_cmd();
        drush_log('Site Verified', 'devshop_log');
    }


    /**
     * Return the inventory file from ansible configuration.
     *
     * @return string
     */
    protected function getAnsibleInventory() {
        if (!$this->inventory) {
            $this->config = $this->getAnsibleConfig();
            if (isset($this->config['inventory'])) {
                $this->inventory = $this->config['inventory'];
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
                $file = @parse_ini_file($path);
                if (is_array($file)) {
                    $this->config_file = $path;
                    return $file;
                }
            }
        }
        return array();
    }
}
