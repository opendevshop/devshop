<?php
/**
 * @file devshop_project.node.inc DevShop Project Node related hooks and 
 * support function include file.
 */

/**
 * Implementation of hook_node_info().
 */
function devshop_projects_node_info() {
  #configuration
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
 * Implements hook_nodeapi()
 *
 * For Platforms and Sites:
 *   $node->project : The context name of the project.
 *   $node->environment: The type of environment this platform or site is:
 *	can be dev, test, or live.
 *   $node->git_branch: The current git branch the project lives on.
 */
function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

  // Project-enabled platforms get
  if ($node->type == 'platform' || $node->type == 'site'){
    
    // On Load: load this object's project and environment type
    if ($op == 'load'){
      $data = db_fetch_array(db_query('SELECT project_nid, env_type AS environment, n.title as project, git_url, git_branch FROM {hosting_devshop_project_object} d LEFT JOIN {node} n ON n.nid = d.project_nid LEFT JOIN {hosting_devshop_project} p ON n.nid = p.nid WHERE object_nid = %d', $node->nid));
      return $data;
    }
    
    // Display Project info
    if ($op == 'view'){
      if (!empty($node->project)){
        $node->content['info']['project'] = array(
	  '#type' => 'item',
	  '#title' => t('Project'),
	  '#value' => l($node->project, "node/$project->project_nid"),
	  '#weight' => -12,
	);
         $node->content['info']['env'] = array(
	  '#type' => 'item',
	  '#title' => t('Environment Type'),
	  '#value' => $node->environment,
	  '#weight' => -11,
        );
      }
    }
    
    // On update or delete, delete the existing records.  We will replace below.
    if ($op = 'update' || $op = 'delete') {
      db_query('DELETE FROM {hosting_devshop_project_object} WHERE object_nid = %d', $node->nid);
    }
    
    // On insert or update, insert records saving this objects project and environment
    if ($op = 'update' || $op = 'insert') {
      if (!empty($node->project) && !empty($node->environment)) {
	// Get the project node by name.
	$project = hosting_context_load($node->project);
	
	// Save to table
	// @TODO: start saving branch here too
	db_query('INSERT INTO {hosting_devshop_project_object} (project_nid, object_nid, object_type, env_type, git_branch) VALUES (%d, %d, "%s", "%s", "%s")', $project->nid, $node->nid, $node->type, $node->environment, $node->git_branch);
      }
    }
  }
}

/**
 * Helper to load a project node by path
 */
function devshop_project_load_by_path($project_path){
  $nid = db_result(db_query('SELECT nid FROM {hosting_devshop_project} WHERE code_path = "%s"', array($project_path)));
  return node_load($nid);
}

/**
 * Implementation of hook_insert().
 */
function devshop_projects_insert($node) {

  db_query("INSERT INTO {hosting_devshop_project} (nid, git_url, code_path, base_url) ".
  	   "VALUES (%d, '%s', '%s', '%s')",
	   $node->nid, $node->git_url, hosting_path_normalize($node->code_path), $node->base_url);

  // Save hosting context
  if ((!$node->old_vid)) {
    hosting_context_register($node->nid, ($node->hosting_name) ? $node->hosting_name : $node->title);
  }
  
  // Create hostmaster task
  hosting_add_task($node->nid, 'verify');
}

/**
 * Implementation of hook_update().
 */
function devshop_projects_update($node) {

  $data = array();
  $data['git_branches'] = $node->git_branches;
  $data['git_tags'] = $node->git_tags;

  db_query("UPDATE {hosting_devshop_project} " .
           "SET git_url = '%s', code_path = '%s', base_url = '%s', data='%s' " .
  	   "WHERE nid = %d",
	   $node->git_url, hosting_path_normalize($node->code_path), 
	   $node->base_url, serialize($data), $node->nid);
  //
  //// If this is a rerry, kick start another devshop-create
  //if (isset($node->retry)) {
  //  $args = array('retry' => 1);
  //
  //  // Create hostmaster task
  //  hosting_add_task($node->nid, 'devshop-create', $args);
  //}
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
  $additions = db_fetch_array(db_query('SELECT git_url, code_path, base_url, install_profile, data ' .
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
    $objects[$project_object->object_type][$project_object->object_nid] = $project_object->env_type;
  }
  $additions['project_objects'] = $objects;
  
  //Project status
  $platforms_ready = TRUE;
  $sites_ready = TRUE;
  $sites_installing = FALSE;
  
  // PLATFORMS STATUS: Determine if all platforms are verified.
  if (isset($additions['project_objects']['platform'])){
    foreach ($additions['project_objects']['platform'] as $nid => $env){
      $platform_nodes[$env] = node_load($nid);
      if ($platform_nodes[$env]->platform_status == 0){
        $platforms_ready = FALSE;
      }
    }
  } else {
    $platforms_ready = FALSE;
  }
  // SITES STATUS: Determine if sites exist and are enabled
  if ($platforms_ready && isset($additions['project_objects']['site'])){
    foreach ($additions['project_objects']['site'] as $nid => $env){
      $site_nodes[$env] = node_load($nid);
      if ($site_nodes[$env]->site_status == 0){
        $sites_ready = FALSE;
        $sites_installing = TRUE;
      }
    }
  } else {
    $sites_ready = FALSE;
  }
  
  // Set status
  $additions['project_status'] = $sites_ready? 'sites_ready': (
    $sites_installing? 'sites_installing': (
      $platforms_ready? 'platforms_ready': 'platforms_verifying'
    )  
  );
  
  return $additions;
}

/**
 * Implementation of hook_view().
 */
function devshop_projects_view($node, $teaser = FALSE, $page = FALSE) {
  modalframe_parent_js();

  $node->content['info'] = array(
    '#prefix' => '<div id="hosting-platform-info">',
    '#suffix' => '</div>'
  );

  $node->content['info']['git_url'] = array(
    '#type' => 'item',
    '#title' => t('Git URL'),
    '#value' => $node->git_url,
    '#weight' => -10
  );

  $node->content['info']['code_path'] = array(
    '#type' => 'item',
    '#title' => t('Code path'),
    '#value' => filter_xss($node->code_path),
    '#weight' => -8
  );
  $url = 'http://' . $node->base_url;
  $node->content['info']['base_url'] = array(
    '#type' => 'item',
    '#title' => t('Base URL'),
    '#value' => l($url, $url),
    '#weight' => -10
  );
  
  if (!empty($node->install_profile)){
    $node->content['info']['install_profile'] = array(
      '#type' => 'item',
      '#title' => t('Install profile'),
      '#value' => ($node->install_profile),
      '#weight' => -8
    );
  }

  $rows = array();
  if (isset($node->project_objects['site'])) {
    foreach($node->project_objects['site'] as $nid => $env) {
      $site = node_load($nid);
      $platform = node_load($site->platform);
      if ($site->site_status == -2 && $platform->platform_status == -2) {
        continue;
      }
      $row = array();
      if ($site->site_status != -2) {
        $url = "http://$site->title";
        $row[] = l($url, $url, array('attributes' => array('target' => '_blank')));
        $row[] = l('Site', "node/$site->nid");
      }
      else {
        // Site if deleted, just show the platform name
        $row[] = $platform->title;
        $row[] = '';
      }
      $row[] = l('Platform', "node/$site->platform");
      if ($site->site_status != -2) {
  $row[] = l('Download Database', "$url/admin/status/");
      }
      else {
        $row[] = '';
      }
      $row[] = l('Commit Log', "node/$site->platform/gitlog");
      $rows[] = $row;
    }
    $header = array();
    $table = theme('table', $header, $rows);

    $node->content['sites'] = array(
      '#type' => 'fieldset',
      '#title' => t('Project Environments'),
      '#weight' => 12,
    );
    $node->content['sites']['table'] = array(
      '#type' => 'item',
      '#value' => $table,
    );
  }
  if ($node->project_status == 'sites_ready'){
    $node->content['sites']['add_platform'] = array(
      '#type' => 'item',
      '#value' => l(t('Create and add additional platforms'), 
      	       	      "hosting/projects/platform/create/$node->nid"),
    );
  }

  $node->content['sites']['delete_project'] = array(
    '#type' => 'item',
    '#value' => l("Delete this project", "hosting/projects/delete/$node->nid")
  );

  //Tasks    
  $node->content['tasks_view'] = array(
    '#type' => 'item',
    //'#value' => devshop_projects_hosting_task_table($node),
    '#value' => hosting_task_table($node),
    '#prefix' => '<div id="hosting-task-list">',
    '#suffix' => '</div>',
    '#weight' => 10
  );
  $settings['hostingTaskRefresh'] = array(
    'nid' => $node->nid,
    'changed' => $node->changed,
  );
  drupal_add_js($settings, 'setting');
  drupal_add_js(drupal_get_path('module','hosting_task') . '/hosting_task.js');  

  // MAIN DISPLAY
  $node->content['devshop'] = array(
    '#type' => 'fieldset',
    '#weight' => 100,
  );
  // Site install form
  $node->content['devshop']['install'] = array(
    '#value' => drupal_get_form('devshop_projects_install_sites_form', $node),
    '#type' => 'markup',
  );
  return $node;
}

/*
 * Callback that is invoked when the user points the brower to
 * hosting/projects. It displays a nice tabulated list of projects
 * and cool thing you can do with them.
 */
function devshop_projects_projects_view() {

  $header = array('Projects', 'Platforms', 'Sites');

  $r = db_query("SELECT * FROM {hosting_devshop_project}");
  $rows = array();

  while(($proj = db_fetch_object($r))) {
    $node = node_load($proj->nid);
    $row = array();
    $row[] = l($node->title, "node/$proj->nid");
    $row[] = db_result(db_query("SELECT COUNT(*) FROM {hosting_devshop_project_object} WHERE project_nid = %d AND object_type='platform'", $node->nid));
    $row[] = db_result(db_query("SELECT COUNT(*) FROM {hosting_devshop_project_object} WHERE project_nid = %d AND object_type='site'", $node->nid));
    $rows[] = $row;
  }

  $output = theme('table', $header, $rows, array('class' => 'hosting-table'));
  $output .= l(t('Create a new project?'), 'node/add/project');
  return $output;
}
