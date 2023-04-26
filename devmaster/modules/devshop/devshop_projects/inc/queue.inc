<?php

///**
// * Implements hook_hosting_queues()
// */
//function devshop_projects_hosting_queues() {
//  $items['deploy'] = array(
//    'type' => 'batch',
//    'name' => t('Deploy Queue'),
//    'description' => t('Deploy Queue: Runs a Deploy task on projects configured to use "Manual Deployment" for environments that are not set to "Disable Deploy on Commit".'),
//    'total_items' => count(devshop_projects_get_deploy_queue_environments()),
//    'frequency' => strtotime("5 minutes", 0),
//    'singular' => t('environment'),
//    'plural' => t('environments'),
//  );
//  return $items;
//}
//
///**
// * Get the environments to be pulled in the queue.
// *
// * @param $limit
// *   Limit to a maximum of this number of platforms.
// * @return
// *   An array of site nodes that have a pull queue enabled.
// *
// */
//function devshop_projects_get_deploy_queue_environments() {
//
//  $results = db_query("SELECT nid FROM {hosting_devshop_project}")->fetchCol();
//  $environments = array();
//  foreach ($results as $nid) {
//    $node = node_load($nid);
//    if (!isset($node->project)) {
//      continue;
//    }
//    $project = $node->project;
//    if (isset($project->settings->deploy) && !empty($project->settings->deploy['queue'])){
//      foreach ($project->environments as $environment) {
//        if ($project->settings->git['refs'][$environment->git_ref] == 'branch' && $environment->settings->pull_disabled == 0) {
//          $environments[] = $environment;
//        }
//      }
//    }
//  }
//  return $environments;
//}
//
///**
// * Implements hosting_QUEUE_TYPE_queue().
// */
//function hosting_deploy_queue($count) {
//  $environments = devshop_projects_get_deploy_queue_environments($count);
//  foreach ($environments as $environment) {
//    $args = array();
//    $args['git_ref'] = $environment->git_ref;
//    $args['update'] = $environment->settings->deploy['update'];
//    $args['revert'] = $environment->settings->deploy['revert'];
//    $args['cache'] = $environment->settings->deploy['cache'];
//    hosting_add_task($environment->site, 'devshop-deploy', $args);
//  }
//}
