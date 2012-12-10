<?php
/**
 * @file devshop_projects.drush.inc
 * Keeping things organized.  All code that is really run via drush commands
 * will go here.
 */
/**
 * Implementation of hook_post_hosting_TASK_TYPE_task
 *
 * If this is a new platform created by a project, we will create a site here.
 *
 * @TODO: This is used only for creating the new site when a "branch platform"
 * is created... we should save something in the task or platform node so we
 * don't have to use this logic.
 */
function devshop_projects_post_hosting_verify_task($task, $data) {

  // We only case about platforms.
  if ($task->ref->type != 'platform') {
    return;
  }

  // Get objects
  $nid = $task->ref->nid;
  $platform = node_load($nid);

  // If this platform isn't in a project, bail.
  if (empty($platform->project_nid)){
    drush_log('[DEVSHOP] No project found for this platform.', 'notice');
    return;
  }

  // Get the project
  $project = node_load($platform->project_nid);

  // If the project doesn't have an install profile chosen yet, bail.
  if (empty($project->install_profile)){
    drush_log('[DEVSHOP] No install profile found for this platform\'s project.', 'notice');
    return;
  }

  // If the project has a site already, bail.
  $sites = array_flip($project->project_objects['site']);
  $sites = hosting_get_sites_by_status($platform->nid, HOSTING_SITE_ENABLED);
  if (!empty($sites)){
    drush_log('[DEVSHOP] Platform already has a site.', 'notice');
    return;
  }

  // live. Let's create a site based off of this platform.
  drush_log('[DEVSHOP] Platform verified.  Creating your site.');
  $site_node = devshop_projects_create_site($project, $platform, $platform->environment);

  drush_log('[DEVSHOP] Site Created! Please wait for it to be installed. !link', array('!link' => l('node/' . $site_node->nid)));

}
