<?php
/**
 * @file devshop_project.node.inc
 *
 * DevShop Project Node related hooks and support functions.
 *
 */

/**
 * Implementation of hook_node_info().
 */
function devshop_projects_node_info() {
  $types["project"] =  array(
    "type" => 'project',
    "name" => 'DevShop Project',
    "module" => 'devshop_projects',
    "has_title" => TRUE,
    "title_label" => t('Project Codename'),
    "description" => t('Create a project'),
    "has_body" => 0,
    "body_label" => '',
    "min_word_count" => 0
  );
  return $types;
}


/**
 * Implementation of hook_load().
 *
 * Loads data for project and environments.
 *
 * @param node
 *    Node object
 */
function devshop_projects_load($node) {

  // Load the "hosting context". The unique name in the aegir system.
  $additions['hosting_name'] = 'project_' . db_result(db_query("SELECT name AS hosting_name FROM {hosting_context} WHERE nid = %d", $node->nid));

  // Load project data
  $project = db_fetch_object(db_query('SELECT * FROM {hosting_devshop_project} WHERE nid = %d', $node->nid));

  // Load up all project settings, merging db table and serialized data.
  $project->data = unserialize($project->data);

  // @TODO: Create a good status system.
  // Set status
  // $project->status = devshop_project_status((object) (array_merge($additions, (array) $node)));

  // Load Environments
  $query = db_query("SELECT * FROM {hosting_devshop_project_environment} WHERE project_nid = %d ORDER BY environment", $node->nid);
  $environments = array();
  while ($environment = db_fetch_object($query)) {
    $environment->data = unserialize($environment->data);
    $environments[$environment->environment] = $environment;
  }
  $project->environments = $environments;

  // Make project name available.
  $project->name = $node->title;

  // Save project object be available at $node->project.
  $additions['project'] = $project;

  return $additions;
}

/**
 * Implementation of hook_insert().
 *
 *   1. Saves data into our table.
 *   2. Saves a hosting context name.
 *   3. Adds a "Verify" task for this project.
 *
 *  @see hosting_platform_insert()
 */
function devshop_projects_insert($node) {

  if (!isset($node->no_verify)) {
    hosting_add_task($node->nid, 'verify');
  }
  
  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $node->project->git_url;
  $info->code_path = hosting_path_normalize($node->project->code_path);
  $info->drupal_path = hosting_path_normalize($node->project->drupal_path);
  $info->base_url = $node->project->base_url;
  $info->install_profile = $node->project->install_profile;

  // Save serialized data, minus environments
  $data = $node->project;
  unset($data->environments);
  $info->data = serialize($data);

  drupal_write_record('hosting_devshop_project', $info);

  // Save hosting context
  if (!$node->old_vid) {
    hosting_context_register($node->nid, ($node->hosting_name) ? $node->hosting_name : $node->title);
  }

  // If using environment aliases... save them.
  devshop_project_save_domain_aliases($node);

  // Save Environment records.
  if (!empty($node->project->environments)) {
    foreach ($node->project->environments as $env => $environment){
      $info = new stdClass();
      $info->project_nid = $node->nid;
      $info->environment = $env;
      $info->git_ref = $environment['git_ref'];
      $info->site = $environment['site'];
      $info->platform = $environment['platform'];
      $info->data = serialize($environment);

      drupal_write_record('hosting_devshop_project_environment', $info);
    }
  }
}

/**
 * Implementation of hook_update().
 *
 *  1. Updates our table.
 *  2. Adds a "Verify" task for this project.
 *
 */
function devshop_projects_update($node) {

  if (!$node->no_verify) {
    hosting_add_task($node->nid, 'verify');
  }

  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $node->project->git_url;
  $info->code_path = hosting_path_normalize($node->project->code_path);
  $info->drupal_path = hosting_path_normalize($node->project->drupal_path);
  $info->base_url = $node->project->base_url;
  $info->install_profile = $node->project->install_profile;

  $environments = $node->project->environments;

  // Save serialized data, minus environments
  $data = $node->project;
  unset($data->environments);
  $info->data = serialize($data);

  drupal_write_record('hosting_devshop_project', $info, 'nid');

  // If using environment aliases... save them.
  devshop_project_save_domain_aliases($node);



  // Save Environment records.
  if (!empty($environments)) {
    // Delete existing environment records
    db_query('DELETE FROM {hosting_devshop_project_environment} WHERE project_nid = %d', $node->nid);

    // Save each environment
    foreach ($environments as $env => $environment){
      $info = new stdClass();
      $info->project_nid = $node->nid;
      $info->environment = $env;
      $info->git_ref = $environment['git_ref'];
      $info->site = $environment['site'];
      $info->platform = $environment['platform'];

      unset($environment->data);
      $info->data = serialize($environment);

      if (drupal_write_record('hosting_devshop_project_environment', $info)){
        drupal_set_message(t('Environment %name saved.', array('%name' => $env)));
      }
      else {
        drupal_set_message(t('Something went wrong saving environment %name.', array('%name' => $env)), 'error');
      }
    }
  }
}


/**
 * Helper to add a domain alias to a site node. Makes sure not to add the same
 * alias twice.
 */
function devshop_project_save_domain_aliases($project_node) {
  if (module_exists('hosting_alias')){
    if (!empty($project_node->live_domain) && $project_node->live_domain_aliases) {
      foreach ($project_node->project->environments as $env => $details){
        $domain = "$env.$project_node->live_domain";
        $site_node = node_load($details['site_nid']);
        if (array_search($domain, $site_node->aliases) === FALSE) {
          $site_node->aliases[] = $domain;
          node_save($site_node);
        }
      }
    }
    // If live_domain_aliases is checked, remove them from the nodes.
    elseif (!$project_node->live_domain_aliases){
      foreach ($project_node->project->environments as $env => $details){
        $domain = "$env.$project_node->live_domain";
        $site_node = node_load($details['site_nid']);
        if (!empty($site_node->aliases)){
          $i = array_search($domain, $site_node->aliases);
          if ($i !== FALSE) {
            unset($site_node->aliases[$i]);
            node_save($site_node);
          }
        }
      }
    }
  }
}

/**
 * Implementation of hook_delete().
 */
function devshop_projects_delete($node) {

  db_query('DELETE FROM {hosting_devshop_project} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {hosting_devshop_project_environment} WHERE project_nid = %d', $node->nid);

  // @TODO: Remove once table is deleted.
  db_query('DELETE FROM {hosting_devshop_project_object} WHERE project_nid = %d', $node->nid);

  hosting_context_delete($node->nid);
}



/**
 * Implementation of hook_nodeapi()
 *
 * Handle project information for Platforms and Sites:
 *   $node->environment: All information about this environment.
 */
function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

  // Load environment info into platforms and sites.
  if ($op == 'load' && $node->type == 'platform' || $node->type == 'site') {
    $additions = array();
    $additions['environment'] = db_fetch_object(db_query("SELECT e.*, n.title AS project FROM {hosting_devshop_project_environment} e LEFT JOIN {node} n ON e.project_nid = n.nid WHERE {$node->type} = %d", $node->nid));

    $additions['project'] = db_fetch_object(db_query('SELECT * FROM {hosting_devshop_project} WHERE nid = %d', $additions['environment']->project_nid));

    $additions['environment']->data = unserialize($additions['environment']->data);
    $additions['project']->data = unserialize($additions['project']->data);

    return $additions;
  }
}

// @TODO: Remove.  I believe this is all cruft from back things were a bit more decoupled.
//
//    if ($op == 'load') {
//
//      $data = db_fetch_array(db_query('SELECT d.project_nid, environment, n.title as project, git_url, git_branch, d.drupal_path, d.clone_nid FROM {hosting_devshop_project_object} d LEFT JOIN {node} n ON n.nid = d.project_nid LEFT JOIN {hosting_devshop_project} p ON n.nid = p.nid WHERE object_nid = %d', $node->nid));
//
//      $data[$node->type . "_nid"] = $node->nid;
//      return $data;
//    }
//
//    // On insert or update, insert records saving this objects project and environment
//    if ($op == 'update' || $op == 'insert') {
//      //Special case for migrate site. Is need copy the information to the new site.
//      if ($op == 'insert' && $node->type == 'site' && $node->import && !$node->verified) {
//        $platform = node_load($node->platform);
//        $node->project = $platform->project;
//        $node->project_nid = $platform->project_nid;
//        $node->environment = $platform->environment;
//        $node->git_branch = $platform->git_branch;
//        $node->drupal_path = $platform->drupal_path;
//        $node->clone_nid = $platform->clone_nid;
//      }
//      if (!empty($node->project) && !empty($node->environment)) {
//        // Get the project node by name.
//        $project = hosting_context_load($node->project);
//
//        // Save to table
//        $data = new stdClass();
//        $data->project_nid = $project->nid;
//        $data->object_nid = $node->nid;
//        $data->object_type = $node->type;
//        $data->environment = $node->environment;
//        $data->git_branch = $node->git_branch;
//        $data->drupal_path = $node->drupal_path;
//
//        //Site used for cloned the new site or platorm
//        if (isset($node->clone_nid)) {
//          $data->clone_nid = $node->clone_nid;
//        }
//
//        if ($op == 'insert') {
//          drupal_write_record('hosting_devshop_project_object', $data);
//        }
//        else {
//          drupal_write_record('hosting_devshop_project_object', $data, 'object_nid');
//        }
//
//        //If we are updating or inserting a platform, update all sites with the correct git_branch and environment
//        if ($node->type == 'platform') {
//          $result = db_query("SELECT nid FROM {hosting_site} WHERE platform = %d", $node->nid);
//          while ($site = db_fetch_object($result)) {
//            db_query('UPDATE {hosting_devshop_project_object} SET git_branch = "%s", environment = "%s", drupal_path = "%s" WHERE object_nid = %d', $node->git_branch, $node->environment, $node->drupal_path, $site->nid);
//          }
//        }
//
//        // If we are updating or inserting sites, check for live domain aliases.
//        $live_domain_alias = "{$node->environment}.{$project->live_domain}";
//
//        // If project should use live_domain_aliases, and site does not have the alias already...
//        if ($node->type == 'site' && $project->live_domain_aliases && array_search($live_domain_alias, $node->aliases) === FALSE) {
//          $node->aliases[] = $live_domain_alias;
//        }
//      }
//    }
//  }
//}
