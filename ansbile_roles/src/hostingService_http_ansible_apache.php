<?php
/**
 * @file
 * Hosting service classes for the Hosting web server module.osting service classes for the Hosting web server module.
 */



class hostingService_http_ansible_apache extends hostingService_http_public {
    public $type = 'ansible_apache';

    protected $has_restart_cmd = TRUE;

    function form(&$form) {
        parent::form($form);
        $form['note'] = array(
            '#markup' => t('Your web server will be configured automatically.'),
            '#prefix' => '<p>',
            '#suffix' => '</p>',
        );
    }
}
