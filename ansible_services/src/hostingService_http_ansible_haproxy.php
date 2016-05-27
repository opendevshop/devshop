<?php
/**
 * @file
 * Hosting service classes for the Hosting web server module.osting service classes for the Hosting web server module.
 */



class hostingService_http_ansible_haproxy extends hostingService_http_cluster
{
    public $type = 'ansible_haproxy';
    public $service = 'http';
    public $name = 'Ansible: HAProxy';

    function form(&$form)
    {
        parent::form($form);
    }

    function load()
    {
        parent::load();
        $this->roles = $this->getRoleNames();

        // Transform the chosen web servers into ansible variables.
        // See https://github.com/geerlingguy/ansible-role-haproxy#role-variables
        $web_servers = node_load_multiple($this->web_servers);
        foreach ($web_servers as $server_node) {
            $this->ansible_vars['haproxy_backend_servers'][] = array(
              'name' => $server_node->title,
              'address' => array_shift($server_node->ip_addresses),
            );
        }
    }

    function insert() {
        parent::insert();
    }
    function delete() {
        parent::delete();
    }
    function update() {
        parent::update();
    }

    /**
     * The list of ansible roles that this service depends on.
     *
     * @return array
     */
    function getRoles() {
        return array(
            'geerlingguy.haproxy'
        );
    }

    /**
     * The list of ansible roles that this service depends on.
     *
     * @return array
     */
    function getRoleNames() {
        $roles = $this->getRoles();
        foreach ($roles as $role) {
            if (isset($role['name'])) {
                $names[] = $role['name'];
            }
            else {
                $names[] = $role;
            }
        }
        return $names;
    }
}
