<?php
/**
 * @file
 * Template file for a project's alias file.
 */
print "<?php // Automatically written by devshop when a project is verified. Do Not Edit. \n";

foreach ($environments as $name => $environment) {

  if ($environment['site_status'] != 1) {
    continue;
  }
  $alias = array(
    'root' => $environment['root'],
    'uri' => $environment['uri'],
    'remote_host' => d('web_server')->remote_host,
    'remote_user' => d('web_server')->script_user,
    'path-aliases' => array(
      '%files' =>  "sites/{$environment['uri']}/files"
    ),
  );

  $export = var_export($alias, TRUE);
  ?>

$aliases['<?php print $name; ?>'] = <?php print $export; ?>;

  <?php
}
?>
