<?php

/**
 * @param $project
 */
function devshop_project_aliases($project) {
  $output = '<?php
';

  foreach ($project->environments as $name => $environment) {

    $remote_user = 'aegir';
    $remote_host = $environment->remote_host;
    $root = $environment->root;
    $uri = $environment->uri;
    $file_path = "sites/{$environment->uri}/files";

    $output .= <<<PHP
\$aliases['$name'] = array(
  'root' => '$root',
  'uri' => '$uri',
  'remote-user' => '$remote_user',
  'remote-host' => '$remote_host',
  'path-aliases' => array(
    '%files' => '$file_path',
  ),
);

PHP;
  }

  return $output;
}


/**
 * Downloads the drush aliases for this site.
 * @param $project
 */
function devshop_project_drush_aliases_page($node) {
  $project = $node->project;
  $filename = $project->name . '.aliases.drushrc.php';
  header("Content-Disposition: attachment; filename='$filename'");
  print devshop_project_aliases($project);
}
