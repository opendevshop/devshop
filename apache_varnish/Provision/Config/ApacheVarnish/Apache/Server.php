<?php

/**
 * Server config file for Apache + Varnish.
 *
 * This configuration file replaces the Apache server configuration file, but
 * inside the template, the original file is once again included.
 */
class Provision_Config_ApacheVarnish_Apache_Server extends Provision_Config_Http_Server {
  public $template = 'server_varnish.tpl.php';

  function write() {
    parent::write();
  }

  function process() {
    drush_log('[DEVSHOP] PROCESS!', 'ok');
    parent::process();
    $this->data['extra_config'] = "# DevShop Varnish! \n";
    $this->data['extra_config'] = "\n";
    $this->data['extra_config'] = "# Extra configuration from modules:\n";
    $this->data['extra_config'] .= join("\n", drush_command_invoke_all('provision_apache_server_config', $this->data));
  }
}
