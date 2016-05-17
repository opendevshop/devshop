<?php
/**
 * @file
 * Hosting service classes for the Hosting web server module.osting service classes for the Hosting web server module.
 */



class hostingService_load_ansible_haproxy extends hostingService_http_cluster
{
    public $type = 'ansible_haproxy';
    public $name = 'Load Balancer';

    function form(&$form)
    {
        parent::form($form);
    }


    /**
     * Load ansible variables.
     */
    function load()
    {
        parent::load();
        $this->roles = $this->getRoles();
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
}
