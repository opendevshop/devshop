<?php

/**
 *
 */
function devshop_project_logs_page($project_node, $environment) {

  $environment = $project_node->project->environments[$environment];

  $env = t('Environment Tasks Log');
  $output = "<h3>{$environment->name} <small>{$env}</small></h3>";
  $output .= views_embed_view('hosting_task_log', 'page_1', $environment->site);

  return $output;
}
