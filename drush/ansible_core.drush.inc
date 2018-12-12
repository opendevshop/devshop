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
function ansible_core_drush_init()
{
  if (file_exists(d('@hostmaster')->root . '/sites/all/composer/vendor/autoload.php')) {
    require_once( d('@hostmaster')->root . '/sites/all/composer/vendor/autoload.php');
  }
}
