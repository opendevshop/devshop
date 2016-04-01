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
    public $has_port = TRUE;

    function form(&$form) {
        parent::form($form);
        $form['note'] = array(
            '#markup' => t('Your MySQL server will be configured automatically.'),
            '#prefix' => '<p>',
            '#suffix' => '</p>',
        );
        $form['db_user'] = array(
            '#type' => 'value',
            '#value' => isset($this->db_user)? $this->db_user: 'server_master_root',
        );
        $form['db_passwd'] = array(
            '#type' => 'value',
            '#value' => isset($this->db_passwd)? $this->db_passwd: user_password(32),
        );
    }

    function load() {
        parent::load();

        // Get server_master's IP address.
        $name = 'server_master';
        $server_master_ip = db_query("SELECT ip_address FROM {hosting_ip_addresses} ip LEFT JOIN {hosting_context} h ON ip.nid = h.nid WHERE name = :name", array(':name' => $name))
            ->fetchField();

        // Load into ansible variables
        // If the server has mysql, load the user/password as variables
        // If there is a server master, we are assuming that it wants database access.
        // This is how aegir database service works.
        $user = array();
        $user['name'] = $this->db_user;
        $user['password'] = $this->db_passwd;
        $user['host'] = $server_master_ip;
        $user['priv']  = '*.*:ALL,GRANT';

        $this->ansible_vars['mysql_users'][] = $user;
        $this->ansible_vars['mysql_port'] = $this->port;
    }
}
