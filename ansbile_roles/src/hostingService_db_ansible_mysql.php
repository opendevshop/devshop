<?php
/**
 * @file
 * Provide the hosting serivce classes for database integration.
 */

/**
 * A MySQL specific db service implementation class.
 */
class hostingService_db_ansible_mysql extends hostingService_db {
    public $type = 'ansible_mysql';
    public $has_port = FALSE;

    function form(&$form) {
        parent::form($form);
        $form['note'] = array(
            '#markup' => t('Your MySQL server will be configured automatically.'),
            '#prefix' => '<p>',
            '#suffix' => '</p>',
        );
        $form['db_user'] = array(
            '#type' => 'value',
            '#value' => isset($this->db_user)? isset($this->db_user): 'server_master_root',
        );
        $form['db_passwd'] = array(
            '#type' => 'value',
            '#value' => isset($this->db_passwd)? isset($this->db_passwd): user_password(16),
        );
    }
}
