<?php

use Symfony\Component\Process\Process;

/**
 * A Process class:
 */
class Provision_Service_Process extends Provision_Service {
}

/**
 * A Process class:
 */
class Provision_Service_Process_Process extends Provision_Service_Process {
  public $service = 'Process';

  function process($command, $cwd = null, $label = 'Process', $env = array(), $log_output = TRUE) {

    if (provision_is_local_host($this->server->remote_host)) {
      return provision_process($command, $cwd, $label, $env, $log_output);
    }
    else {
      return provision_process('ssh ' . drush_get_option('ssh-options', '-o PasswordAuthentication=no') . ' ' . $this->server->script_user . '@' . $this->server->remote_host . ' ' . $command, $cwd, $label, $env);
    }
  }
}
