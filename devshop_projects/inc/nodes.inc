<?php
/**
 * @file devshop_project.node.inc
 *
 * DevShop Project Node related hooks and support functions.
 *
 * @TODO: Add default http and db servers to project nodes.
 */

/**
 * Implementation of hook_node_info().
 */
function devshop_projects_node_info() {
  $types["project"] =  array(
    "type" => 'devshop_project',
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
 * Implementation of hook_nodeapi()
 *
 * Handle project information for Platforms and Sites:
 *   $node->project : The context name of the project.
 *   $node->environment: The type of environment this platform or site is:
 *	can be dev, test, or live.
 *   $node->git_branch: The current git branch the project lives on.
 */
function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

  if ($node->type == 'project') {
    // Settings are added here, since $git_branches hadn't been added to the
    // node yet in devshop_projects_load().
    $settings = module_invoke_all('devshop_project_settings', $node);

    foreach ($settings as $setting_name => $setting){
      $setting_node_type = $setting['#node_type'];
      foreach ((array) $node->project_objects[$setting_node_type] as $nid => $environment_name){
        $object = node_load($nid);
        $node->settings[$environment_name][$setting_name] = $object->{$setting_name};
      }
    }
  }

  // Project-enabled platforms get
  if ($node->type == 'platform' || $node->type == 'site'){

    // On Load: load this object's project and environment type
    if ($op == 'load'){
      $data = db_fetch_array(db_query('SELECT project_nid, environment, n.title as project, git_url, git_branch FROM {hosting_devshop_project_object} d LEFT JOIN {node} n ON n.nid = d.project_nid LEFT JOIN {hosting_devshop_project} p ON n.nid = p.nid WHERE object_nid = %d', $node->nid));
      return $data;
    }

    // On update or delete, delete the existing records.  We will replace below.
    if ($op == 'update' || $op == 'delete') {
      db_query('DELETE FROM {hosting_devshop_project_object} WHERE object_nid = %d', $node->nid);
    }

    // On insert or update, insert records saving this objects project and environment
    if ($op == 'update' || $op == 'insert') {
      if (!empty($node->project) && !empty($node->environment)) {
        // Get the project node by name.
        $project = hosting_context_load($node->project);

        // Save to table
        db_query('INSERT INTO {hosting_devshop_project_object} (project_nid, object_nid, object_type, environment, git_branch) VALUES (%d, %d, "%s", "%s", "%s")', $project->nid, $node->nid, $node->type, $node->environment, $node->git_branch);
        
        //If we are updating or inserting a platform, update all sites with the correct git_branch and environment
        if ($node->type == 'platform') {
          $result = db_query("SELECT nid FROM {hosting_site} WHERE platform = %d", $node->nid);
          while ($site = db_fetch_object($result)) {
            db_query('UPDATE {hosting_devshop_project_object} SET git_branch = "%s", environment = "%s" WHERE object_nid = %d', $node->git_branch, $node->environment, $site->nid);
          } 
        }
      }
    }
  }
}

/**
 * Helper to load a project node by code path.
 */
function devshop_project_load_by_path($project_path){
  $nid = db_result(db_query('SELECT nid FROM {hosting_devshop_project} WHERE code_path = "%s"', array($project_path)));
  return node_load($nid);
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
  
  $data = array();
  $data['git_branches'] = $node->git_branches;
  $data['git_tags'] = $node->git_tags;

  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $node->git_url;
  $info->code_path = hosting_path_normalize($node->code_path);
  $info->base_url = $node->base_url;
  $info->install_profile = $node->install_profile;
  $info->live_domain = $node->live_domain;
  $info->data = serialize($data);

  drupal_write_record('hosting_devshop_project', $info);

  // Save hosting context
  if ((!$node->old_vid)) {
    hosting_context_register($node->nid, ($node->hosting_name) ? $node->hosting_name : $node->title);
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
  $data = array();
  
  // Branches and tags get saved upon verify.
  $data['git_branches'] = $node->git_branches;
  $data['git_tags'] = $node->git_tags;

  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $node->git_url;
  $info->code_path = hosting_path_normalize($node->code_path);
  $info->base_url = $node->base_url;
  $info->install_profile = $node->install_profile;
  $info->live_domain = $node->live_domain;
  $info->data = serialize($data);

  drupal_write_record('hosting_devshop_project', $info, 'nid');
}

/**
 * Implementation of hook_delete().
 */
function devshop_projects_delete($node) {

  db_query('DELETE FROM {hosting_devshop_project} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {hosting_devshop_project_object} WHERE project_nid = %d', $node->nid);

  hosting_context_delete($node->nid);
}

/**
 * Implementation of hook_load().
 *
 * @param node
 *    Node object
 */
function devshop_projects_load($node) {
  $additions = db_fetch_array(db_query('SELECT * ' .
                                        'FROM {hosting_devshop_project} ' .
                                        'WHERE nid = %d', $node->nid));
  $hosting_name = db_fetch_array(db_query("SELECT name AS hosting_name FROM {hosting_context} WHERE nid = %d", $node->nid));
  if (is_array($hosting_name) && is_array($additions)) {
    $hosting_name['hosting_name'] = 'project_' . $hosting_name['hosting_name'];
    $additions += $hosting_name;
  }
  $data = unserialize($additions['data']);
  $additions['git_branches'] = $data['git_branches'];
  $additions['git_tags'] = $data['git_tags'];

  $query = db_query("SELECT * FROM {hosting_devshop_project_object} WHERE project_nid = %d", $node->nid);

  $objects = array();
  while($project_object = db_fetch_object($query)) {
    // Only load if site or platform is enabled.
    // @TODO: We should load all.  Let other code hide by status.
    $site_node = node_load($project_object->object_nid);
    $objects[$project_object->object_type][$project_object->object_nid] = $project_object->environment;
  }
  $additions['project_objects'] = $objects;

  // Set status
  $additions['project_status'] = devshop_project_status((object) (array_merge($additions, (array) $node)));

  // Settings are added in devshop_projects_nodeapi

  return $additions;
}

