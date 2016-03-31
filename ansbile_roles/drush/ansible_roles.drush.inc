<?php

/**
 * @file
 *   An ansible_roles of the provision service API.
 *
 *  Declares a new service type and a basic implementation of it.
 *  It matches the same service definition in the hosting front end.
 */

/**
 * Implements hook_drush_init().
 */
function ansible_roles_drush_init()
{
  require_once('Provision/Service/db/ansible_mysql.php');
  require_once('Provision/Service/http/ansible_apache.php');
}