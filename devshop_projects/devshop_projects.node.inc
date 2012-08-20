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
 * hook_nodeapi()
 */
function devshop_projects_nodeapi(&$node, $op, $a3 = null) {

  if ($node->type == 'platform'){
    // On insert of a platform, determine project and save to hosting_devshop_project_objects
    // @TODO: This is how we save the project path at the moment because
    //  the platform node is coming from hosting-import...
    if ($op == 'insert') {
      $path = explode('/', $node->publish_path); 
      $environment = array_pop($path);
      $project_path = implode('/', $path);
      
      $project = devshop_project_load_by_path($project_path);
  
      // Save to table
      db_query('INSERT INTO {hosting_devshop_project_object} (project_nid, object_nid, object_type, env_type) VALUES (%d, %d, "%s", "%s")', $project->nid, $node->nid, $node->type, $environment);
    }
  }
  
  //Platforms and Sites
  if ($node->type == 'platform' || $node->type == 'site'){
    
    // Load Project info
    if ($op == 'load'){
      $data = db_fetch_array(db_query('SELECT project_nid, env_type AS project_environment FROM {hosting_devshop_project_object} WHERE object_nid = %d', $node->nid));
      return $data;
    }
    
    // Display Project info
    if ($op == 'view'){
      $project = node_load($node->project_nid);
      $node->content['info']['project'] = array(
	'#type' => 'item',
	'#title' => t('Project'),
	'#value' => l($project->title, "node/$project->nid"),
	'#weight' => -12
      );
       $node->content['info']['env'] = array(
	'#type' => 'item',
	'#title' => t('Environement Type'),
	'#value' => $node->project_environment,
	'#weight' => -11
      );
      return $data;
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
//  _devshop_projects_insert_sites($node);

  // Save hosting context
  if ((!$node->old_vid)) {
    hosting_context_register($node->nid, ($node->hosting_name) ? $node->hosting_name : $node->title);
  }
  
  // Create hostmaster task
  hosting_add_task($node->nid, 'devshop-create');
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
  $additions = db_fetch_array(db_query('SELECT git_url, code_path, base_url, install_profile ' .
                                        'FROM {hosting_devshop_project} ' .
                                        'WHERE nid = %d', $node->nid));
  $hosting_name = db_fetch_array(db_query("SELECT name AS hosting_name FROM {hosting_context} WHERE nid = %d", $node->nid));
  if (is_array($hosting_name) && is_array($additions)) {
    $hosting_name['hosting_name'] = 'project_' . $hosting_name['hosting_name'];
    $additions += $hosting_name;
  }
  
  $query = db_query("SELECT * FROM {hosting_devshop_project_object} WHERE project_nid = %d", $node->nid);

  $objects = array();
  while($project_object = db_fetch_object($query)) {
    $objects[$project_object->object_type][$project_object->object_nid] = $project_object->env_type;
  }
  $additions['project_objects'] = $objects;
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
  foreach($node->project_objects as $type => $objects) {
    $header = array($type ."s");
    $rows = array();
    foreach($objects as $nid => $object) {
      $object = node_load($nid);
      $row = array(l($object->title, "node/$object->nid"));
      $rows[] = $row;
      
    }
    $table = theme('table', $header, $rows);

    $node->content['info'][$type] = array(
      '#type' => 'item',
      '#value' => $table,
    );    
  }
  
  // MAIN DISPLAY
  
  //If there are no sites yet, we must display the install form.
  if (!isset($node->project_objects['site'])){
    
    // Site install form
    $node->content['install'] =array(
      '#type' => 'item',
      '#value' => drupal_get_form('devshop_projects_install_sites_form', $node),
    );
    
  }

  return $node;
}

/*
 * Callback that is invoked when the user points the brower to
 * hosting/projects. It displays a nice tabulated list of projects
 * and cool thing you can do with them.
 */

function devshop_projects_projects_view() {

  $header = array('Projects', 'Sites');

  $r = db_query("SELECT * FROM {hosting_devshop_project}");
  $rows = array();

  while(($proj = db_fetch_object($r))) {
    $node = node_load($proj->nid);
    $row = array();
    $row[] = l($node->title, "node/$proj->nid");
    $row[] = db_result(db_query("SELECT COUNT(*) FROM {hosting_devshop_project_object} WHERE project_nid = %d", $node->nid));
    $rows[] = $row;
  }

  $output = theme('table', $header, $rows, array('class' => 'hosting-table'));
  $output .= l(t('Create a new project?'), 'node/add/project');
  return $output;
}
