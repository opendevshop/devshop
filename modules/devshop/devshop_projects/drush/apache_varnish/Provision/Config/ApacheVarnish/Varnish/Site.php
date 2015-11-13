<?php

/**
 * Virtual host config file for Apache + SSL.
 *
 * This file is created in addition to the existing virtual host, 
 * and includes some extra directives.
 */
class Provision_Config_Apache_Varnish_Site extends Provision_Config_Http_Site {

  public $template = 'vhost_varnish.tpl.php';

  // The template file to use when the site has been disabled.
  public $disabled_template = 'vhost_varnish_disabled.tpl.php';

  function process() {
    parent::process();
    $this->data['extra_config'] = "# DEVSHOP VARNISH! \n";
    $this->data['extra_config'] = "\n";
    $this->data['extra_config'] = "# Extra configuration from modules:\n";
    $this->data['extra_config'] .= join("\n", drush_command_invoke_all('provision_apache_vhost_config', $this->uri, $this->data));
  }
}
