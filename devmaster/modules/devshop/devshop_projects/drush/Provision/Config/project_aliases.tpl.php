<?php
/**
 * @file
 * Template file for a project's alias file.
 */
print "<?php // Automatically written by devshop when a project is verified. Do Not Edit. \n";

foreach ($project['environments'] as $name => $environment) {

  if ($environment['site_status'] != 1) {
    continue;
  }
  $alias = array(
    'root' => $environment['root'],
    'uri' => $environment['uri'],
    'remote-host' => d('web_server')->remote_host,
    'remote-user' => d('web_server')->script_user,
    'path-aliases' => array(
      '%files' =>  "sites/{$environment['uri']}/files"
    ),
  );

  $export = var_export($alias, TRUE);
  ?>

$aliases['<?php print $name; ?>'] = <?php print $export; ?>;

  <?php
}
foreach ($project['settings']['aliases'] as $name => $remote_alias) {
  $export = var_export($remote_alias, TRUE);
  ?>

$aliases['<?php print $name; ?>'] = <?php print $export; ?>;

  <?php
}
?>
