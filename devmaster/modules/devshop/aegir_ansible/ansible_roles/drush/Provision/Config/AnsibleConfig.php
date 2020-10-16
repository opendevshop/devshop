<?php

/**
 * A basic configuration file class.
 *
 * Just a file containing the value passed to us.
 */
class Provision_Config_AnsibleConfig extends Provision_Config {
  /**
   * Template file to load. In the same directory as this class definition.
   */
  public $template = 'ansible.cfg.tpl.php';

  function __construct($context = '@none', $data = array()) {
    $this->data['roles_path'] = variable_get('aegir_ansible_roles_path', '/var/aegir/roles');
    parent::__construct($context, $data);
  }

    /**
   * Where the file generated will end up.
   *
   * It is extremely important that this path is only made up of information
   * relative to this class, and does not use drush_get_option or the d() accessor.
   */
  function filename() {
    return  $_SERVER['HOME'] . '/.ansible.cfg';
  }
}
