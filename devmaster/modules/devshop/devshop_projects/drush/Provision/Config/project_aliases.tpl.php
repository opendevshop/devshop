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
  // Tell drush to inherit from the provision site alias record.
  $alias = array(
    'parent' => '@' . $environment['uri']
  );

  // If web server is not server master, add "remote host and user.
  if (d($environment['drush_alias'])->platform->web_server->name != '@server_master') {
    $alias['remote-host'] = d($environment['drush_alias'])->platform->web_server->remote_host;
    $alias['remote-user'] = d($environment['drush_alias'])->platform->web_server->script_user;
  }

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
