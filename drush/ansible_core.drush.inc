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
  $autoloader = drush_get_option('hostmaster_composer_autoload_path');
  if (file_exists($autoloader . '/autoload.php')) {
    require_once($autoloader . '/autoload.php');
  }
}

/**
 * Implements drush_hook_pre_hosting_task()
 */
function drush_ansible_core_pre_hosting_task() {

  $task =& drush_get_context('HOSTING_TASK');
  $task->options['hostmaster_composer_autoload_path'] = DRUPAL_ROOT . '/' . variable_get('composer_manager_vendor_dir');

}
