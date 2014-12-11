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
    $uri = $environment->default_domain;

    $output .= <<<PHP
\$aliases['$name'] = array(
  'root' => '$root',
  'uri' => '$uri',
  'remote-user' => '$remote_user',
  'remote-host' => '$remote_host',
);

PHP;
  }

  return $output;
}