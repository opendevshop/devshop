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
function hosting_create_environment($project, $new_environment_name, $new_environment_branch = NULL, $source_environment = NULL, $settings = NULL, $action = 'create', $git_remote_url = NULL, $create_site = TRUE) {
  $settings = (object) $settings;
  global $user;

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

  // Always prepare platform node. That part is required.
  $platform = devshop_prepare_platform_node($project, $new_environment_name, $new_environment_branch, $settings->web_server, $git_remote_url);

  // Create site node, unless blocked by setting $create_site = FALSE.
  if ($create_site) {
    $site = new stdClass();
    $site->type = 'site';
    $site->title = devshop_environment_url($project, $new_environment_name);
    $site->status = 1;
    $site->uid = $user->uid;
    $site->name = $user->name;
    $site->client = HOSTING_DEFAULT_CLIENT;
    $site->settings = $settings;

    // Attach a new platform node.
    $site->platform_node = $platform;
    $site->db_server = $settings->db_server;

    // Save the site node, along with the platform.
    // This is possible thanks to the patch in https://www.drupal.org/node/2824731
    if ($site = node_submit($site)) {
      node_save($site);
    }
  }
  else {
    // If site doesn't get saved, platform doesn't either, so save it directly.
    if ($platform = node_submit($platform)) {
      node_save($platform);
    }
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

  // If $source_environment is set, and the environment by that name exists, create a new environment.
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

    // If the source environment was installed from a profile, we have to change
    // "install_method" so that it installs by cloning the source site, not by
    // the "profile" install_method.
    if ($source_environment->settings->install_method['method'] == 'profile') {
      $environment->settings->install_method = array(
        'method' => 'clone',
        'clone_source' => $source_environment->system_alias
      );
    }
    else {

      // Use same install configuration as the source environment.
      $environment->settings->install_method = $source_environment->settings->install_method;
    }

    if ($action == 'fork') {
      $environment->settings->branch_to_fork = $source_environment->git_ref;
    }
  }
  else {
    // Next, add the environment record.
    $environment = new stdClass();
    $environment->name = $new_environment_name;
    $environment->site = $site->nid?: NULL;
    $environment->platform = $platform->nid;

    // Use settings passed in to this function.
    $environment->settings = $settings;

    // Save profile to install_method.
    $environment->settings->install_method['method'] = 'profile';
    $environment->settings->install_method['profile'] = $project->install_profile;

  }

  // Get platform verify task
  if (isset($site->nid)) {
    $tasks = hosting_get_tasks('rid', $site->nid);
    $environment->last_task = current($tasks)->nid;
  }
  else {
    $environment->last_task = 0;
  }

  $environment->project_nid = $project->nid;

  db_insert('hosting_devshop_project_environment')
    ->fields(array(
      'project_nid' => $environment->project_nid,
      'name' => $environment->name,
      'site' => $environment->site,
      'platform' => $environment->platform,
      'settings' => serialize($environment->settings),
      'last_task' => $environment->last_task,
    ))
    ->execute();

  module_invoke_all('devshop_post_environment_create', $environment);
  return $environment;
}
