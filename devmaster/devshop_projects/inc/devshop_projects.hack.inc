<?php
/**
 * Replacement menu callback for hosting_task_ajax_list
 */
function devshop_projects_hosting_task_ajax_list($node) {
  if ($node->type == 'project') {
    module_load_include('tasks.inc', 'devshop_projects');
    $return['markup'] = devshop_projects_hosting_task_table($node);
    $return['changed'] = $node->changed;
    drupal_json($return);
    exit();
  } else {
    hosting_task_ajax_list($node);
  }
}

/**
 * Replacement menu callback for hosting_js_page().
 *
 * This is used for Project Tasks.  Since the tasks are really tasks on the site nodes,
 * we have to modify the JS call to modify what node gets passed in.
 */
function devshop_projects_hosting_js_page(){
  $args = func_get_args();
  $nid = &$args[1];
  $task = &$args[2];

  // Get task type from $task argument... For whatver reason, they don't make it an argument
  list($aegir_context, $task_type) = explode('_', $task);
  
  // If this is coming from a project page:
  if ($aegir_context == 'project'){
    // Change the task object from project to site
    $task = str_replace('project', 'site', $task);
    
    // Load the project
    $project = node_load($nid);
    $sites = array_flip($project->project_objects['site']);
    
    // Change the NID based on the task type
    if ($task_type == 'devshop-commit') {
      $nid = $sites['dev'];
    } elseif ($task_type == 'devshop-sync') {
      $nid = $sites['test'];
    } elseif ($task_type == 'devshop-pull') {
      $nid = $sites['live'];
    }
  }
  
  $output .= call_user_func_array('hosting_js_page', $args);
  return $output;
}
/**
 * A replacement for hosting_task_table, to allow us to add
 * tasks from other nodes.
 */
function devshop_projects_hosting_task_table($node) {
  $output = '';

  $headers[] = t('Task');
  $headers[] = array(
    'data' => t('Actions'),
    'class' => 'hosting-actions',
  );

  $tasklist = hosting_task_fetch_tasks($node->nid);
  if ($node->project_status != 'sites_ready'){
    return;
  }
  
  // Get tasklists for all sites
  $tasks = array();
  foreach ($node->project_objects['site'] as $nid => $env){
    $site_nodes[$env] = node_load($nid);
    $tasks[$env] = hosting_task_fetch_tasks($nid);
  }
  
  // Add our own specific tasks
  $tasklist['devshop-commit'] = $tasks['dev']['devshop-commit'];
  $tasklist['devshop-sync'] = $tasks['test']['devshop-sync'];
  $tasklist['devshop-pull'] = $tasks['live']['devshop-pull'];

  // Enhance titles
  $tasklist['devshop-commit']['title'] .= ' on ' . l($site_nodes['dev']->title, 'http://'. $site_nodes['dev']->title, array('attributes' => array('target' => '_blank')));
  $tasklist['devshop-sync']['title'] .= ' on ' . l($site_nodes['test']->title, 'http://'. $site_nodes['test']->title, array('attributes' => array('target' => '_blank')));
  $tasklist['devshop-pull']['title'] .= ' on ' . l($site_nodes['live']->title, 'http://'. $site_nodes['live']->title, array('attributes' => array('target' => '_blank')));

  // Override some
  unset($tasklist['devshop-create']['task_permitted']);
  unset($tasklist['devshop-create']['nid']);

  unset($tasklist['devshop-platform-create']['task_permitted']);
  unset($tasklist['devshop-platform-create']['nid']);
  
  foreach ($tasklist as $task => $info) {
    $row = array();

    if (!isset($info['nid']) && !$info['task_permitted']) {
      // just don't show those tasks, since we'll not be able to run them
      continue;
    }

    $row['type'] = array(
      'data' => $info['title'],
      'class' => 'hosting-status',
    );
    $actions = array();

    if (isset($info['task_status']) && ($info['task_status'] == 0)) {
      $actions['cancel'] = _hosting_task_button(t('Cancel'), sprintf("hosting/tasks/%d/cancel", $info['nid']), t("Cancel the task and remove it from the queue"), 'hosting-button-stop', !$info['task_permitted']);
    }
    else {
      $actions['run'] = _hosting_task_button(t('Run'), sprintf("node/%d/%s_%s", $node->nid, $node->type, $task), $info['description'], 'hosting-button-run', $info['task_permitted'], $info['dialog']);
    }

    $actions['log'] = _hosting_task_button(t('Log'), 'node/' . $info['nid'], t("Display the task log"), 'hosting-button-log', isset($info['nid']) &&  user_access('access task logs'), TRUE, FALSE);
    $row['actions'] = array(
      'data' => implode('', $actions), 
      'class' => 'hosting-actions',
    );

    $rows[] = array(
      'data' => $row,
      'class' => $info['class'],
    );
  }
  $output .= theme('table', $headers, $rows, array('class' => 'hosting-table'));
  return $output;
}