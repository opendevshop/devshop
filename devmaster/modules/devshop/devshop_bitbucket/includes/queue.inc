<?php
/**
 * @file queue.inc
 * Contains hooks to create a Hosting Queue...
 */

/**
 * Implements hook_hosting_queues().
 */
function devshop_bitbucket_hosting_queues() {
  $items['bitbucket'] = array(
    'type' => 'batch',
    'name' => t('BitBucket Pull Requests'),
    'description' => t('BitBucket Polling: Keep environments up to date with latest code and maintain PR environments.'),
    'total_items' => 1,
    'frequency' => strtotime("1 minute", 0),
    'singular' => t('project'),
    'plural' => t('projects'),
  );
  return $items;
}

/**
 * Implements hosting_QUEUE_TYPE_queue().
 *
 * Provides the `hosting-bitbucket` drush command.
 *
 * This is run by cron, via the hosting queues system.
 */
function hosting_bitbucket_queue($count) {
//
//  // Load all published projects.
//  $query = db_select('hosting_devshop_project', 'p');
//  $query->join('node', 'n', 'p.nid = n.nid');
//  $query
//    ->fields('p', array('nid'))
//    ->condition('n.status', NODE_PUBLISHED)
//    ->execute();
//
//  $projects = $query->execute()->fetchAll();
//
//  // Look through all projects...
//  foreach ($projects as $project) {
//    $project_node = node_load($project->nid);
//    $project = $project_node->project;
//
//    drush_log("Checking project {$project->name}...", "success");
//
//    // Check ones that have PR envs enabled
//    if (!empty($project->settings->bitbucket['pull_request_environments'])) {
//      drush_log("Project {$project->name} is configured to maintain environments for all pull requests. Preparing environments...", "success");
//      $message = devshop_bitbucket_create_all_pr_environments($project_node);
//      drush_log($message, "success");
//    }
//  }
//
//  print "Queue done: \n";
//  return TRUE;
}

