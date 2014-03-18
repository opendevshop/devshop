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
 * Implementation of hook_nodeapi()
 *
 * Handle project information for Platforms and Sites:
 *   $node->project : The context name of the project.
 *   $node->environment: The type of environment this platform or site is:
 *	can be dev, test, or live.
 *   $node->git_branch: The current git branch the project lives on.
 */
function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

  if ($node->type == 'project' && isset($node->git_branches)) {
    // Settings are added here, since $git_branches hadn't been added to the
    // node yet in devshop_projects_load().
    $settings = module_invoke_all('devshop_project_settings', $node);

    foreach ($settings as $setting_name => $setting) {
      $setting_node_type = $setting['#node_type'];
      foreach ((array) $node->project_objects[$setting_node_type] as $nid => $environment_name) {
        $object = node_load($nid);
        $node->settings[$environment_name][$setting_name] = $object->{$setting_name};
      }
    }
  }

  // Project-enabled platforms get
  if ($node->type == 'platform' || $node->type == 'site') {
    // On Load: load this object's project and environment type
    if ($op == 'load') {
      $data = db_fetch_array(db_query('SELECT d.project_nid, environment, n.title as project, git_url, git_branch, d.drupal_path, d.clone_nid FROM {hosting_devshop_project_object} d LEFT JOIN {node} n ON n.nid = d.project_nid LEFT JOIN {hosting_devshop_project} p ON n.nid = p.nid WHERE object_nid = %d', $node->nid));
      $data[$node->type . "_nid"] = $node->nid;
      return $data;
    }

    // On insert or update, insert records saving this objects project and environment
    if ($op == 'update' || $op == 'insert') {
      //Special case for migrate site. Is need copy the information to the new site.
      if ($op == 'insert' && $node->type == 'site' && $node->import && !$node->verified) {
        $platform = node_load($node->platform);
        $node->project = $platform->project;
        $node->project_nid = $platform->project_nid;
        $node->environment = $platform->environment;
        $node->git_branch = $platform->git_branch;
        $node->drupal_path = $platform->drupal_path;
        $node->clone_nid = $platform->clone_nid;
      }
      if (!empty($node->project) && !empty($node->environment)) {
        // Get the project node by name.
        $project = hosting_context_load($node->project);

        // Save to table
        $data = new stdClass();
        $data->project_nid = $project->nid;
        $data->object_nid = $node->nid;
        $data->object_type = $node->type;
        $data->environment = $node->environment;
        $data->git_branch = $node->git_branch;
        $data->drupal_path = $node->drupal_path;

        //Site used for cloned the new site or platorm
        if (isset($node->clone_nid)) {
          $data->clone_nid = $node->clone_nid;
        }

        if ($op == 'insert') {
          drupal_write_record('hosting_devshop_project_object', $data);
        }
        else {
          drupal_write_record('hosting_devshop_project_object', $data, 'object_nid');
        }

        //If we are updating or inserting a platform, update all sites with the correct git_branch and environment
        if ($node->type == 'platform') {
          $result = db_query("SELECT nid FROM {hosting_site} WHERE platform = %d", $node->nid);
          while ($site = db_fetch_object($result)) {
            db_query('UPDATE {hosting_devshop_project_object} SET git_branch = "%s", environment = "%s", drupal_path = "%s" WHERE object_nid = %d', $node->git_branch, $node->environment, $node->drupal_path, $site->nid);
          } 
        }

        // If we are updating or inserting sites, check for live domain aliases.
        $live_domain_alias = "{$node->environment}.{$project->live_domain}";

        // If project should use live_domain_aliases, and site does not have the alias already...
        if ($node->type == 'site' && $project->live_domain_aliases && array_search($live_domain_alias, $node->aliases) === FALSE) {
          $node->aliases[] = $live_domain_alias;
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
  
  $data = $node->project_settings;
  $data['git_branches'] = $node->git_branches;
  $data['git_tags'] = $node->git_tags;

//  $data['live_alias'] = $node->live_alias;
//  $data['live_domain'] = $node->live_domain;
//  $data['live_domain_aliases'] = $node->live_domain_aliases;

  $data['live_environment'] = $node->live_environment;
  $data['live_environment_alias'] = $node->live_environment_alias;

  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $node->git_url;
  $info->code_path = hosting_path_normalize($node->code_path);
  $info->drupal_path = hosting_path_normalize($node->drupal_path);
  $info->base_url = $node->base_url;
  $info->install_profile = $node->install_profile;
  $info->data = serialize($data);

  drupal_write_record('hosting_devshop_project', $info);

  // Save hosting context
  if ((!$node->old_vid)) {
    hosting_context_register($node->nid, ($node->hosting_name) ? $node->hosting_name : $node->title);
  }

  // If using environment aliases... save them.
  devshop_project_save_domain_aliases($node);
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
  $info->git_url = $node->git_url;
  $info->code_path = hosting_path_normalize($node->code_path);
  $info->drupal_path = hosting_path_normalize($node->drupal_path);
  $info->base_url = $node->base_url;
  $info->install_profile = $node->install_profile;

  // Everything in $form['project_settings'] gets serialized and saved.
  $info->data = serialize($node->project_settings);

  drupal_write_record('hosting_devshop_project', $info, 'nid');

  // If using environment aliases... save them.
  devshop_project_save_domain_aliases($node);

}


/**
 * Helper to add a domain alias to a site node. Makes sure not to add the same
 * alias twice.
 */
function devshop_project_save_domain_aliases($project_node) {
  if (module_exists('hosting_alias')){
    if (!empty($project_node->live_domain) && $project_node->live_domain_aliases) {
      foreach ($project_node->settings as $env => $details){
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
      foreach ($project_node->settings as $env => $details){
        $domain = "$env.$project_node->live_domain";
        $site_node = node_load($details['site_nid']);
        $i = array_search($domain, $site_node->aliases);
        if ($i !== FALSE) {
          unset($site_node->aliases[$i]);
          node_save($site_node);
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
  $additions = db_fetch_array(db_query('SELECT * FROM {hosting_devshop_project} WHERE nid = %d', $node->nid));
  $hosting_name = db_fetch_array(db_query("SELECT name AS hosting_name FROM {hosting_context} WHERE nid = %d", $node->nid));
  if (is_array($hosting_name) && is_array($additions)) {
    $hosting_name['hosting_name'] = 'project_' . $hosting_name['hosting_name'];
    $additions += $hosting_name;
  }

  // Load up all project settings
  $additions['project_settings'] = unserialize($additions['data']);

  // Make most common attributes available.
  $additions['git_branches'] = $additions['project_settings']['git_branches'];
  $additions['git_tags'] = $additions['project_settings']['git_tags'];

  $query = db_query("SELECT * FROM {hosting_devshop_project_object} WHERE project_nid = %d", $node->nid);

  $objects = array();
  while($project_object = db_fetch_object($query)) {
    // Only load if site or platform is enabled.
    $object_node = node_load($project_object->object_nid);
    
    if (($object_node->type == 'site' && $object_node->site_status == HOSTING_SITE_ENABLED) ||
      ($object_node->type == 'platform' && $object_node->platform_status == HOSTING_PLATFORM_ENABLED)
        ){
      $objects[$project_object->object_type][$project_object->object_nid] = $project_object->environment;

    }
  }
  $additions['project_objects'] = $objects;

  // Set status
  $additions['project_status'] = devshop_project_status((object) (array_merge($additions, (array) $node)));

  // Environments Array
  // @TODO: Remove the above code, after refactoring out of the rest of the system.
  $query = db_query("SELECT * FROM {hosting_devshop_project_object} WHERE project_nid = %d ORDER BY environment", $node->nid);

  $environments = array();
  while ($obj = db_fetch_object($query)) {
    $object_node = node_load($obj->object_nid);
    
    if (($object_node->type == 'site' && $object_node->site_status != HOSTING_SITE_DELETED) ||
      ($object_node->type == 'platform' && $object_node->platform_status != HOSTING_PLATFORM_DELETED)
        ){
      $environments[$obj->environment][$obj->object_type] = $obj->object_nid;
    }
  }
  $additions['environments'] = $environments;
  
  // Settings are added in devshop_projects_nodeapi

  return $additions;
}

