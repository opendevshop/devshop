<?php

/**
 * The server_data service class.
 */
class Provision_Service_ansible_roles_available extends Provision_Service {
  public $service = 'ansible_roles_available';

  /**
   * Add the needed properties to the server context.
   */
  static function subscribe_server($context) {
    $context->setProperty('ansible_roles_available');
  }
}
