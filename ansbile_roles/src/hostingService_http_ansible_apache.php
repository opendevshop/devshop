<?php
/**
 * @file
 * Hosting service classes for the Hosting web server module.osting service classes for the Hosting web server module.
 */



class hostingService_http_ansible_apache extends hostingService_http
{
    public $type = 'ansible_apache';

    protected $has_restart_cmd = FALSE;

    function form(&$form)
    {
        parent::form($form);
        $form['note'] = array(
            '#markup' => t('Your web server will be configured automatically.'),
            '#prefix' => '<p>',
            '#suffix' => '</p>',
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
