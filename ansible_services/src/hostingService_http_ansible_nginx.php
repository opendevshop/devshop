<?php
/**
 * @file
 * HostingService class to enable NGINX via Ansible.
 *
 *
 */



class hostingService_http_ansible_nginx extends hostingService_http_nginx_ssl
{
    public $type = 'ansible_nginx';
    public $name = 'Ansible: NGINX SSL';

    function form(&$form)
    {
        parent::form($form);
        $form['note'] = array(
            '#markup' => t('Your web server will be configured automatically.'),
            '#prefix' => '<p>',
            '#suffix' => '</p>',
        );
        $form['restart_cmd'] = array(
            '#type' => 'value',
            '#value' => isset($this->restart_cmd)? $this->restart_cmd: '',
        );
    }


    /**
     * Load nginx ansible variables.
     */
    function load()
    {
        parent::load();
        $this->ansible_vars['aegir_user_authorized_keys'] = variable_get('devshop_public_key', '');
        $this->roles = $this->getRoleNames();
    }

    /**
     * The list of ansible roles that this service depends on.
     *
     * @return array
     */
    function getRoles() {
        return array(
            'opendevshop.aegir-nginx',
            'geerlingguy.php' => array(
                'name' => 'geerlingguy.php',
                'version' => '3.7.0',
            ),
            'geerlingguy.php-mysql' => array(
              'name' => 'geerlingguy.php-mysql',
              'version' => '2.0.1',
            ),
            'geerlingguy.composer'
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
