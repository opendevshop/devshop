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
  $name = db_result(db_query("SELECT name AS hosting_name FROM {hosting_context} WHERE nid = %d", $node->nid));
  $additions['hosting_name'] = 'project_' . $name;

  // Load project data
  $project = db_fetch_object(db_query('SELECT * FROM {hosting_devshop_project} WHERE nid = %d', $node->nid));

  // Load up all project settings.
  $project->name = $name;
  $project->settings = (object) unserialize($project->settings);

  // Load Environments
  // @TODO: Remove environments where the site has been deleted.
  $query = db_query("
    SELECT
        e.*,
        s.status as site_status,
        p.status as platform_status,
        http.title as web_server,
        db.title as db_server
      FROM {hosting_devshop_project_environment} e
      LEFT JOIN {hosting_site} s ON e.site = s.nid
      LEFT JOIN {hosting_platform} p ON e.platform = p.nid
      LEFT JOIN {node} http ON p.web_server = http.nid
      LEFT JOIN {node} db ON s.db_server = db.nid
      WHERE project_nid = %d
      ORDER BY name;   ", $node->nid);
  $environments = array();
  while ($environment = db_fetch_object($query)) {

    // Skip this if it is not enabled.
    if ($environment->site_status == HOSTING_SITE_DELETED && $environment->platform_status == HOSTING_PLATFORM_DELETED) {
      continue;
    }

    // Save to environments array
    $environment->settings = (object) unserialize($environment->settings);

    // Environments URL:
    // @TODO: Use aegir generated link
    $environment->url = 'http://' . $environment->name . '.' . $project->name . '.' . $_SERVER['HTTP_HOST'];
    $environment->git_ref_type = $project->settings->git['refs'][$environment->git_ref];

    // Environment version
    $iid = db_result(db_query("SELECT iid FROM {hosting_package_instance} i left join {hosting_package} p on p.nid=i.package_id WHERE p.package_type='platform' AND i.rid=%d", $environment->platform));
    $release = hosting_package_instance_load($iid);
    $environment->version = $release->version;

    // Get extra server info
    $environment->servers['http'] = array(
      'nid' => $environment->settings->web_server,
      'name' => $environment->web_server
    );

    // HTTP Server
    $environment->servers['db'] = array(
      'nid' => $environment->settings->db_server,
      'name' => $environment->db_server
    );

    // Save to project environments collection.
    $environments[$environment->name] = $environment;
  }
  $project->environments = $environments;

  // Make project name and status available.
  $project->name = $node->title;
  $project->status = $node->status;

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
  $info->settings = serialize($node->project->settings);

  drupal_write_record('hosting_devshop_project', $info);

  // Save hosting context
  if (!$node->old_vid) {
    hosting_context_register($node->nid, ($node->hosting_name) ? $node->hosting_name : $node->title);
  }

  // @TODO: The wizard always creates the project before the environments.
  // Not sure if we need this, but we might to enable importing via drush.
  // Save Environment records.
  if (!empty($node->project->environments)) {
    foreach ($node->project->environments as $name => $environment){
      // Ensure correct data types
      $environment = (object) $environment;
      $environment->settings = (array) $environment->settings;

      $info = new stdClass();
      $info->project_nid = $node->nid;
      $info->name = $name;
      $info->git_ref = $environment->git_ref;
      $info->site = $environment->site;
      $info->platform = $environment->platform;

      // Remove primary settings from settings array before saving.
      unset($environment->settings['git_ref']);
      unset($environment->settings['site']);
      unset($environment->settings['platform']);
      $info->settings = serialize($environment->settings);

      // Save environment record.
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

  $project = (object) $node->project;
  $project->settings = (object) $project->settings;

  $info = new stdClass();
  $info->nid = $node->nid;
  $info->git_url = $project->git_url;
  $info->code_path = hosting_path_normalize($project->code_path);
  $info->drupal_path = hosting_path_normalize($project->drupal_path);
  $info->base_url = $project->base_url;
  $info->install_profile = $project->install_profile;

  // Save serialized data, minus environments
  $info->settings = serialize($project->settings);

  // Write project record.
  drupal_write_record('hosting_devshop_project', $info, 'nid');

  // Save Environment records.
  if (!empty($project->environments )) {
    // Delete existing environment records
    db_query('DELETE FROM {hosting_devshop_project_environment} WHERE project_nid = %d', $node->nid);

    // Save each environment
    foreach ($project->environments as $name => $environment){
      // Ensure correct data types
      $environment = (object) $environment;
      $environment->name = $name;
      $environment->settings = (array) $environment->settings;

      $info = new stdClass();
      $info->project_nid = $node->nid;
      $info->name = $name;
      $info->git_ref = $environment->git_ref;
      $info->site = $environment->site;
      $info->platform = $environment->platform;

      // Remove primary settings from settings array before saving.
      // @TODO: These values shouldn't show up anymore. Once tested, remove.
      unset($environment->settings['git_ref']);
      unset($environment->settings['site']);
      unset($environment->settings['platform']);
      $info->settings = serialize($environment->settings);

      // Save environment record.
      drupal_write_record('hosting_devshop_project_environment', $info);

      // Save aegir objects
      if (!empty($environment->site)){
        $site = node_load($environment->site);
        $platform = node_load($environment->platform);

        // Save to array so we can apply values for settings.
        $objects = array();
        $objects['site'] = &$site;
        $objects['platform'] = &$platform;

        $settings_info = module_invoke_all('devshop_project_settings', $node);

        // Go through each setting, applying it to either the site or the platform.
        foreach ($settings_info as $setting_name => $setting_element) {
          $node_type = $settings_info[$setting_name]['#node_type'];

          // Set new value only if it has changed.
          $old_value = $environment->orig->settings->{$setting_name};
          if (isset($old_value) && $objects[$node_type]->{$setting_name} != $old_value){
            $objects[$node_type]->{$setting_name} = $environment->settings[$setting_name];
            $objects[$node_type]->needs_save = TRUE;

            // If the database server is changed, a "migrate" task is needed.
            if ($setting_name == 'db_server') {
              $args = array();
              $args['target_platform'] = $site->platform;
              $args['new_uri'] = $site->title;
              $args['new_db_server'] = $environment->settings['db_server'];
              hosting_add_task($site->nid, 'migrate', $args);
            }
          }
        }

        // If git_ref changed, flag needs_save for platform.
        if ($environment->orig->git_ref != $environment->git_ref){
          $platform->needs_save = TRUE;
        }

        // If there's a live domain
        if (!empty($project->settings->live['live_domain'])) {

          // Save the live domain
          // @TODO: Remove the alias from the old "live" environment
          if ($project->settings->live['live_environment'] == $environment->name) {
            if (array_search($project->settings->live['live_domain'], $site->aliases) === FALSE) {
              $site->aliases[] = $project->settings->live['live_domain'];
              $site->needs_save = TRUE;
            }

            // Save www version
            if ($project->settings->live['live_domain_www']) {
              $site->aliases[] = 'www.' . $project->settings->live['live_domain'];
              $site->needs_save = TRUE;
            }
            else {
              $site->aliases = array_diff($site->aliases, array('www' . $project->settings->live['live_domain']));
            }
          }
          // Save other environment aliases if environment_aliases is checked.
          elseif ($project->settings->live['environment_aliases']) {
          $domain = $environment->name . '.' . $project->settings->live['live_domain'];
            if (array_search($domain, $site->aliases) === FALSE) {
              $site->aliases[] = $domain;
              $site->needs_save = TRUE;
            }
          }
        }

        // If environment_aliases is NOT checked, remove them from the nodes.
        elseif (!$project->settings->live['environment_aliases'] && count($project->environments)){
          foreach ($project->environments as $env => $environment){
            $domain = $environment->name . '.' . $project->settings->live['live_domain'];
            if (!empty($site->aliases)){
              $i = array_search($domain, $site->aliases);
              if ($i !== FALSE) {
                unset($site->aliases[$i]);
                $site->needs_save = TRUE;
              }
            }
          }
        }

        // Save site and platform if needed.
        if ($platform->needs_save) {
          node_save($platform);
        }
        if ($site->needs_save) {
          node_save($site);
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

  hosting_context_delete($node->nid);
}



/**
 * Implementation of hook_nodeapi()
 * For site and platform nodes, load project and environment objects.
 *
 * @see devshop_projects_load()
 */
function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

  // Load environment info into platforms and sites.
  if ($op == 'load' && $node->type == 'platform' || $node->type == 'site') {
    $additions = array();
    $result = db_fetch_object(db_query("SELECT project_nid, name FROM {hosting_devshop_project_environment} e WHERE {$node->type} = %d", $node->nid));

    // Load project and environment
    $project_node = node_load($result->project_nid);
    $additions['project'] = $project_node->project;
    $additions['environment'] = $project_node->project->environments[$result->name];
    return $additions;
  }
}
