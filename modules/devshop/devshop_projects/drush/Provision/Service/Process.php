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

  function shell_exec($command) {

    drush_log('PROCESS shell exec!', 'devshop_log');
    if (provision_is_local_host($this->server->remote_host)) {
      return devshop_drush_process(escapeshellcmd($command));
    }
    else {
      return devshop_drush_process('ssh ' . drush_get_option('ssh-options', '-o PasswordAuthentication=no') . ' %s %s', $this->script_user . '@' . $this->remote_host, escapeshellcmd($command));
    }
  }
}
