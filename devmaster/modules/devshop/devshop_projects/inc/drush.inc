<?php

/**
 * @param $project
 */
function devshop_project_aliases($project) {
  $output = '<?php
';
  $aliases = [];
  foreach ($project->environments as $name => $environment) {
    $site = node_load($environment->site);
    $aliases[$name] = [
      'root' => $environment->root,
      'uri' => $environment->uri,
      'host' => $environment->remote_host,
      'user' => 'aegir',
      'options' => [
        'path-aliases' => [
          '%files' => $site->file_public_path,
        ]
      ]
    ];

  }
  $output = \Symfony\Component\Yaml\Yaml::dump($aliases, 8, 2);
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
