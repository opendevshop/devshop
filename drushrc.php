<?php
/**
 * @file drushrc.php
 *
 * This file is base config file for drush, used globally if the devshop drush 
 * bin is used.
 * 
 * For this file to be picked up by calls to `drush`, it must be in one of the 
 * possible drushrc locations.
 *
 * To ensure this file is used in DevShop installations, it is symlinked into
 * the drush source code folder, typically /usr/share/devshop/vendor/drush/drush
 * 
 * It does this using composer post-install-cmd. See 
 *
 * @see composer.json.
 */

// Remember: Drush reads this file from the Drush source path, so __DIR__ == vendor/drush/drush
$options['include']['provision'] = dirname(dirname(dirname(__DIR__))) . '/drupal/provision';
