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
  global $user;

  // Load the "hosting context". The unique name in the aegir system.
  $name = db_result(db_query("SELECT name AS hosting_name FROM {hosting_context} WHERE nid = %d", $node->nid));
  $additions['hosting_name'] = 'project_' . $name;

  // Load project data
  $project = db_fetch_object(db_query('SELECT * FROM {hosting_devshop_project} WHERE nid = %d', $node->nid));

  // Load up all project settings.
  $project->name = $name;
  $project->settings = (object) unserialize($project->settings);

  // Create "refs" array to determine ref type.
  $project->settings->git['refs'] = array();
  foreach ($project->settings->git as $type => $refs) {
    foreach ($refs as $ref) {
      $project->settings->git['refs'][$ref] = ($type == 'branches')? 'branch': 'tag';
    }
  }

  // Git Repo Host
  if (strpos($project->git_url, 'github.com') !== FALSE) {
    $project->git_provider = 'github';
    $project->git_repo_url = strtr($project->git_url, array(
      'git@github.com:' => 'http://github.com/',
      'git://' => 'http://',
      '.git' => '',
    ));
  }
  else {
    $project->git_provider = 'git';
    $project->git_repo_url = '';
  }
  // @TODO: Set git providers for most common here, then add a hook to detect.

  // Load Environments
  // @TODO: Remove environments where the site has been deleted.
  $query = db_query("
    SELECT
        e.*,
        s.status as site_status,
        p.status as platform_status,
        p.publish_path as root,
        http.title as remote_host,
        sn.title as uri,
        sn.title as drush_alias,
        http.title as web_server,
        http.nid as web_server_nid,
        db.title as db_server,
        db.nid as db_server_nid,
        n.title as project_name,
        s.vid,
        sn.title as system_domain,
        a.redirection as redirect_domain
      FROM {hosting_devshop_project_environment} e
      LEFT JOIN {hosting_site} s ON e.site = s.nid
      LEFT JOIN {node} sn ON s.vid = sn.vid
      LEFT JOIN {hosting_platform} p ON e.platform = p.nid
      LEFT JOIN {node} http ON p.web_server = http.nid
      LEFT JOIN {node} db ON s.db_server = db.nid
      LEFT JOIN {node} n ON e.project_nid = n.nid
      LEFT JOIN {hosting_site_alias} a ON a.vid = s.vid
      WHERE project_nid = %d AND
      e.name != ''
      ORDER BY name;   ", $node->nid);
  $environments = array();
  while ($environment = db_fetch_object($query)) {

    // Don't load the environment if the site and/or platform is deleted.
    if ($environment->site_status == HOSTING_SITE_DELETED && $environment->platform_status == HOSTING_PLATFORM_DELETED) {
      continue;
    }
    if ($environment->platform_status == HOSTING_PLATFORM_DELETED) {
      continue;
    }

    // Drush alias
    $environment->drush_alias = '@' . $environment->drush_alias;

    // Unserialize environment settings.
    $environment->settings = (object) unserialize($environment->settings);

    // Get the current git ref type.
    $environment->git_ref_type = $project->settings->git['refs'][$environment->git_ref];

    // The URL that the branch or tag links to.
    if ($project->git_provider == 'github') {
      $environment->git_ref_url = $project->git_repo_url . '/tree/' . $environment->git_ref;
    }
    else {
      $environment->git_ref_url = url("node/$environment->site/logs/commits");
    }



    // Environment Drupal version.
    $iid = db_result(db_query("SELECT iid FROM {hosting_package_instance} i left join {hosting_package} p on p.nid=i.package_id WHERE p.package_type='platform' AND i.rid=%d", $environment->platform));
    $release = hosting_package_instance_load($iid);
    $environment->version = $release->version;

    // @TODO: Make "servers" more abstract so we can add many more.
    // HTTP Server
    $environment->servers['http'] = array(
      'nid' => $environment->web_server_nid,
      'name' => $environment->web_server,
    );

    // DB Server
    $environment->servers['db'] = array(
      'nid' => $environment->db_server,
      'name' => $environment->db_server,
    );

    // Web Server IP
    $server = node_load($environment->web_server_nid);
    $environment->ip_addresses = $server->ip_addresses;

    // Load Solr if present
    if (module_exists('hosting_solr')) {
      $server_nid = db_result(db_query("SELECT server FROM {hosting_solr} WHERE nid=%d", $environment->site));

      $servers = hosting_get_servers('solr');
      if ($server_nid && isset($servers[$server_nid])) {
        $environment->servers['solr'] = array(
          'nid' => $server_nid,
          'name' => $servers[$server_nid],
        );
      }
    }

    // Last commit
    $path = $project->code_path . '/' . $environment->name;
//    $environment->git_current = shell_exec("cd $path; git log --pretty=format:'<b>%h</b> <i>%an</i> <small>%ar</small><br /> <small >%s</small>' HEAD^...HEAD");
    $environment->git_ref_id = shell_exec("cd $path; git log --pretty=format:'%ar' --max-count=1
");
    $environment->git_last = shell_exec("cd $path; git log --pretty=format:'%h %an %ar %s' --max-count=1");

    $environment->git_sha = trim(shell_exec("cd $path; git rev-parse HEAD"));

    // Repo root
    $environment->repo_root = empty($project->drupal_path)? $environment->root: str_replace($project->drupal_path, '', $environment->root);

    // Load encryption status
    if (module_exists('hosting_ssl')){
      $environment->ssl_enabled  = db_result(db_query('SELECT ssl_enabled FROM {hosting_ssl_site} WHERE nid = %d', $environment->site));
    }
    else {
      $environment->ssl_enabled = FALSE;
    }

    // Environments URLs
    // Get aegir "aliases"
    $environment->domains = hosting_alias_get_aliases($environment);
    $environment->domains[] = $environment->system_domain;
    $protocol = $environment->ssl_enabled? 'https://': 'http://';
    if ($environment->redirect_domain) {
      $environment->url = $protocol . $environment->redirect_domain;
    } else {
      $environment->url = $protocol . $environment->system_domain;
    }

    // Save to project environments collection.
    $environments[$environment->name] = $environment;
  }

  // Put live environment at the top.
  if ($project->settings->live['live_environment']) {
    $live_env = $project->settings->live['live_environment'];
    $project->environments = array();
    $project->environments[$live_env] = $environments[$live_env];
    unset($environments[$live_env]);
    $project->environments += $environments;
  }
  else {
    $project->environments = $environments;
  }

  // Make project name and status available.
  $project->name = $node->title;
  $project->status = $node->status;

  // Webhook Status
  // @TODO: Create Hosting Webhooks module.
  // @TODO: Remove devshop_pull.
  module_load_include('inc', 'devshop_pull');
  $project->webhook_url = _devshop_pull_callback_url($node);

  // Save project object be available at $node->project.
  $additions['project'] = $project;

  // Load verification status
  $task = db_fetch_object(db_query('SELECT * FROM {hosting_task} WHERE rid = %d ORDER BY vid DESC', $node->nid));
  $task->task_status = (int) $task->task_status;
  $additions['verify'] = $task;

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
  if (!isset($node->no_verify) || $node->no_verify == FALSE) {
    hosting_add_task($node->nid, 'verify');
  }
}

///**
// * Function for updating a project's alias
// */
//function devshop_project_update_domains($project_node) {
//  $project = $project_node->project;
//  $old_project = $project_node->old->project;
//
//  // If there is a live environment, AND live domain was entered:
//  if ($project->settings->live['live_environment'] && $project->settings->live['live_domain']) {
//
//    // ...and if the Live Domain changed:
//    if ($project->settings->live['live_domain'] != $old_project->settings->live['live_domain']) {
//
//      // Save live domain alias to the live environment
//      $live_environment = $project->settings->live['live_environment'];
//      $live_site_node = node_load($project->environment[$live_environment]->site);
//      devshop_hosting_save_domain($project->settings->live['live_domain'], $live_site_node);
//
//      // Save live domain alias with www prefix.
//      if ($project->settings->live['live_domain_www']) {
//        devshop_hosting_save_domain('www.' . $project->settings->live['live_domain'], $live_site_node);
//      }
//
//      // Delete all the old live domain aliases for all environments, and save new ones.
//      $project_node_loaded = node_load($project_node->nid);
//      foreach ($project_node_loaded->project['environments'] as $name => $environment) {
//        $site_node = node_load($project->environment[$live_environment]->site);
//
//        devshop_hosting_delete_domain($name . "." . $project->settings->live['live_domain'], $site_node);
//        devshop_hosting_save_domain($name . "." . $project->settings->live['live_domain'], $site_node);
//      }
//    }
//
//    // ... or if the Live Environment changed.
//    if ($project->settings->live['live_environment'] != $old_project->settings->live['live_environment']) {
//      $live_environment = $project->settings->live['live_environment'];
//      $live_site_node = node_load($project->environment[$live_environment]->site);
//
//      $old_live_environment = $old_project->settings->live['live_environment'];
//      $old_live_site_node = node_load($project->environment[$old_live_environment]->site);
//
//      // If the live domain also changed, delete the old domain from the old environment).
//      if ($project->settings->live['live_domain'] != $old_project->settings->live['live_domain']) {
//        devshop_hosting_delete_domain($old_project->settings->live['live_domain'], $old_live_site_node);
//
//        // If project had www aliases, delete that alias as well.
//        if ($old_project->settings->live['live_domain_www']) {
//          devshop_hosting_delete_domain("www." . $old_project->settings->live['live_domain'], $old_live_site_node);
//        }
//      }
//      // ... if the live domain did not change, delete the current live domain alias.
//      else {
//        devshop_hosting_delete_domain($project->settings->live['live_domain'], $live_site_node);
//        if ($old_project->settings->live['live_domain_www']) {
//          devshop_hosting_delete_domain("www." . $old_project->settings->live['live_domain'], $live_site_node);
//        }
//      }
//
//      // Save the domain aliases to the new live environment
//      devshop_hosting_save_domain($project->settings->live['live_domain'], $live_site_node);
//      if ($project->settings->live['live_domain_www']) {
//        devshop_hosting_save_domain("www." . $project->settings->live['live_domain'], $live_site_node);
//      }
//    }
//  }
//}

/**
 * Implementation of hook_delete().
 */
function devshop_projects_delete($node) {

  db_query('DELETE FROM {hosting_devshop_project} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {hosting_devshop_project_environment} WHERE project_nid = %d', $node->nid);

  hosting_context_delete($node->nid);
}



/**
vagra * Implementation of hook_nodeapi()
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

  // Load environment info into tasks.
  if ($op == 'load' && $node->type == 'task') {
    $additions = array();
    $result = db_fetch_object(db_query("SELECT nid, title AS name FROM {hosting_devshop_project_environment} e LEFT JOIN node ON e.project_nid = node.nid WHERE site = %d OR platform = %d", $node->rid, $node->rid));

    // Load project and environment
    $additions['project'] = $result;
    return $additions;
  }

  // When a task is created or updated that has a project, redirect to the project.
  if (($op == 'update' || $op == 'insert') && $node->type == 'task') {
    $object = node_load($node->rid);
    if (isset($object->project) && arg(0) == 'node' && arg(1) == $object->nid) {
      drupal_goto("node/{$object->project->nid}");

    }
  }

  // When a platform or site node is deleted, remove the environment record.
  if ($op == 'delete' && ($node->type == 'site' || $node->type == 'platform')) {
    drupal_set_message('Deleted environment.');
    db_query('DELETE FROM {hosting_devshop_project_environment} WHERE %s = %d', $node->type, $node->nid);
  }
}
