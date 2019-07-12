<?php
/**
 * @file create.inc
 * Functions related to the "Create Environment" task.
 */

/**
 * API-level function for creating a new environment.
 *
 * @TODO: Add more options like web server, etc.
 * @param $project
 *   A full project node (for now. @TODO Allow project name (and/or nid) as parameters.
 * @param $new_environment_name
 *   A new name for the environment
 * @param $new_environment_branch
 *   What git branch to track. If FALSE, will not fork.
 * @param $source_environment
 *   If desired, the environment to clone. (Copy the database and create a new branch from)
 */
function hosting_create_environment($project, $new_environment_name, $new_environment_branch = NULL, $source_environment = NULL, $settings = NULL, $action = 'create', $git_remote_url = NULL) {
  $settings = (object) $settings;
  global $user;

  // Create site node.
  $site = new stdClass();
  $site->type = 'site';
  $site->title = devshop_environment_url($project, $new_environment_name);
  $site->status = 1;
  $site->uid = $user->uid;
  $site->name = $user->name;
  $site->client = HOSTING_DEFAULT_CLIENT;

  // Attach a new platform node.
  $site->platform_node = devshop_prepare_platform_node($project, $new_environment_name, $new_environment_branch, $settings->web_server, $git_remote_url);

  // Set db server to the project default, if none was set.
  if (empty($settings->db_server) && !empty($project->settings->default_environment['db_server'])) {
    $settings->db_server = $project->settings->default_environment['db_server'];
  }
  // If no db server is set and there is no default set.
  elseif (empty($settings->db_server)) {
    $servers = hosting_get_servers('db', FALSE);
    reset($servers);
    $settings->db_server = key($servers);
  }

  // If "force servers" is set for the project, force them.
  if ($project->settings->default_environment['force_default_servers']) {
    $settings->db_server = $project->settings->default_environment['db_server'];
    $settings->web_server = $project->settings->default_environment['web_server'];
  }

  // Now that desired servers have been determined, save to their respective nodes.
  $site->db_server = $settings->db_server;

  // Save the site node, along with the platform.
  // This is possible thanks to the patch in https://www.drupal.org/node/2824731
  if ($site = node_submit($site)) {
    node_save($site);
  }

//  // If no new branch specified and fork source is present, set branch to forked environment's branch.
//  if (empty($new_environment_branch) && $source_environment) {
//    $branch = $project->environments[$source_environment]->git_ref;
//  }
//  // If no new branch and fork source not mentioned, return false.
//  elseif (empty($new_environment_branch) && empty($source_environment)) {
//    return FALSE;
//  }
//  else {
//    $branch = $new_environment_branch;
//  }

  // If cloning or forking, check if source environment exists...
  if (isset($project->environments[$source_environment])) {
    $source_environment = $project->environments[$source_environment];

    $environment = new stdClass();
    $environment->name = $new_environment_name;
    $environment->site = $site->nid;

    // Copy settings from source environment.
    $environment->settings = $source_environment->settings;

    // We assume since this is a clone, it does not need to be locked, and it should be auto-pulling.
    // Unlock the environment... @TODO: Should this be a project setting?
    // Make sure pull_disabled isn't on.
    $environment->settings->locked = 0;
    $environment->settings->pull_disabled = 0;

    // Save clone source from source_environment's drush alias.
    $environment->settings->install_method['method'] = 'clone';
    $environment->settings->install_method['clone_source'] = $source_environment->system_alias;

    if ($action == 'fork') {
      $environment->settings->branch_to_fork = $source_environment->git_ref;
    }
  }
  else {
    // Next, add the environment record.
    $environment = new stdClass();
    $environment->name = $new_environment_name;
    $environment->site = $site->nid;

    // Use settings passed in to this function.
    $environment->settings = $settings;

    // Save profile to install_method.
    $environment->settings->install_method['method'] = 'profile';
    $environment->settings->install_method['profile'] = $project->install_profile;

  }

  // Get platform verify task
  $tasks = hosting_get_tasks('rid', $site->platform_node->nid);
  $environment->last_task = current($tasks)->nid;

  db_insert('hosting_devshop_project_environment')
    ->fields(array(
      'project_nid' => $environment->project_nid,
      'name' => $environment->name,
      'site' => $environment->site,
      'settings' => serialize($environment->settings),
      'last_task' => $environment->last_task,
    ))
    ->execute();
  return $environment;
}
