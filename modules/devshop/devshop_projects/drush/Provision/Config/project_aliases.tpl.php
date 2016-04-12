<?php
/**
 * @file
 * Template file for a project's alias file.
 */
print "<?php // Automatically written by devshop. Do Not Edit. \n";

foreach ($environments as $name => $environment) {
  $alias = array(
    'root' => $environment['root'],
    'uri' => $environment['uri'],
    'remote_host' => $environment['remote_host'],
    'remote_user' => d('web_server')->script_user,
  );

  $export = var_export($alias, TRUE);
  ?>
  $aliases['<?php print $name; ?>'] = <?php print $export; ?>;

  <?php
}
?>
