<?php
/**
 * @file
 * Provide the hosting serivce classes for database integration.
 */

/**
 * A MySQL specific db service implementation class.
 */
class hostingService_db_ansible_mysql extends hostingService_db_mysql {
    public $type = 'ansible_mysql';
    public $name = 'Ansible: MySQL';

    public $has_port = TRUE;
    public $ansible_vars = array();

    function form(&$form) {
        parent::form($form);
        $form['note'] = array(
            '#markup' => t('Your MySQL server will be configured automatically.'),
            '#prefix' => '<p>',
            '#suffix' => '</p>',
        );
        $form['db_user'] = array(
            '#type' => 'value',
            '#value' => isset($this->db_user)? $this->db_user: 'aegir_root',
        );
        $form['db_passwd'] = array(
            '#type' => 'value',
            '#value' => isset($this->db_passwd)? $this->db_passwd: user_password(32),
        );
    }

    function load() {
        parent::load();
        $this->roles = $this->getRoleNames();

        // Get server_master's IP address.
        $name = 'server_master';
        $server_master_ip = db_query("SELECT ip_address FROM {hosting_ip_addresses} ip LEFT JOIN {hosting_context} h ON ip.nid = h.nid WHERE name = :name", array(':name' => $name))
            ->fetchField();

        // Get this server's IP address
        if ($this->ip = db_query("SELECT ip_address FROM {hosting_ip_addresses} WHERE nid = :nid", array(':nid' => $this->server->nid))
            ->fetchField()) {

            // Set bind address to 0.0.0.0 so it will listen on all.
            $this->ansible_vars['mysql_bind_address'] = '0.0.0.0';
        }

        // Load into ansible variables
        // If the server has mysql, load the user/password as variables
        // If there is a server master, we are assuming that it wants database access.
        // This is how aegir database service works.
        $user = array();
        $user['name'] = $this->db_user;
        $user['password'] = $this->db_passwd;
        $user['host'] = $server_master_ip? $server_master_ip: 'localhost';
        $user['priv']  = '*.*:ALL,GRANT';

        $this->ansible_vars['mysql_users'][] = $user;
        $this->ansible_vars['mysql_port'] = $this->port;

        // Load or create this server's MySQL Root password and pass to ansible.
        $mysql_root_password_variable_name = "server_root_mysql_passwd_{$this->server->title}";
        $this->ansible_vars['mysql_root_password'] = variable_get($mysql_root_password_variable_name, user_password(64));

        // Save password for later retrieval
        variable_set($mysql_root_password_variable_name, $this->ansible_vars['mysql_root_password']);

        // If we pass the "mysql_root_password_update" variable, it will reset the server's mysql root password

    }

    /**
     * The list of ansible roles that this service depends on.
     *
     * @return array
     */
    function getRoles() {
        return array(
            'opendevshop.users',
            'geerlingguy.mysql',
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
