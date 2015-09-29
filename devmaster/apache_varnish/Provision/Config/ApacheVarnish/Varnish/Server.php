<?php

/**
 * Server config file for Apache + Varnish.
 *
 * This configuration file replaces the Apache server configuration file, but
 * inside the template, the original file is once again included.
 */
class Provision_Config_ApacheVarnish_Varnish_Server extends Provision_Config_Http_Server {
  public $template = 'default.vcl.tpl.php';
  public $description = 'Varnish server default.vcl file.';

  function process() {
    drush_log('[DEVSHOP] PROCESS VARNISH!', 'ok');
    parent::process();
    $this->data['extra_config'] = "# DevShop Varnish! \n";
    $this->data['extra_config'] = "\n";
    $this->data['extra_config'] = "# Extra configuration from modules:\n";
    $this->data['extra_config'] .= join("\n", drush_command_invoke_all('provision_apache_server_config', $this->data));

    $this->data['http_port'] = d()->http_port;
  }

  function write() {
    parent::write();

    $file = 'default.vcl';
    $cmd = sprintf('ln -sf %s %s',
        escapeshellarg($this->data['server']->config_path . '/' . $file),
        escapeshellarg($this->data['server']->aegir_root . '/config/' . $file)
    );

    if ($this->data['server']->shell_exec($cmd)) {
      drush_log(dt("Created symlink for %file on %server", array(
          '%file' => $file,
          '%server' => $this->data['server']->remote_host,
      )), 'ok');

    }

    $this->data['server']->sync($this->filename());
  }

  function filename() {
    return 'varnish.default.vcl';
  }
}
