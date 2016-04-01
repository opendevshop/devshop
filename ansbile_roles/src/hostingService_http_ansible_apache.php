<?php
/**
 * @file
 * Hosting service classes for the Hosting web server module.osting service classes for the Hosting web server module.
 */



class hostingService_http_ansible_apache extends hostingService_http_apache_ssl
{
    public $type = 'ansible_apache';

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
     * Load Apache ansible variables.
     */
    function load()
    {
        parent::load();
        $this->ansible_vars['aegir_user_authorized_keys'] = variable_get('devshop_public_key', '');
    }
}
