<?php
/**
 * @file drushrc.php
 *
 * The purpose of this file is to force global 'drush' commands to include the
 * copy of 'drupal/provision' from the devshop vendor directory.
 *
 * This allows us to remove the "drush" based install of provision, bringing
 * drupal/provision into vendor as a library.
 *
 * @see composer.json.
 */

// Remember: This file is symlinked FROM /usr/share/devshop/drushrc.provision-include.php.
// drush follows the symlink. Call `drush status` and you will see
//   Drush configuration    :  /usr/share/devshop/drushrc.provision-include.php
$provision_path = dirname(__FILE__) . '/vendor/drupal/provision';
if (file_exists($provision_path)) {
  $options['include']['provision'] = $provision_path;
}
