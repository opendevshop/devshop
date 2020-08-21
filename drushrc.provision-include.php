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

// Remember: Drush reads this file from the Drush source path, so __DIR__ == vendor/drush/drush
$options['include']['provision'] = dirname(dirname(dirname(__DIR__))) . '/drupal/provision';
