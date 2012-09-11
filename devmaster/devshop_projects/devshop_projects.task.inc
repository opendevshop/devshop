<?php
/**
 * @file devshop_project.task.inc DevShop Project Task related hooks and 
 * support function include file.
 */

/**
 * Implementation of hook_hosting_tasks()
 */
function devshop_projects_hosting_tasks() {
  $tasks = array();
  $tasks['project']['devshop-create'] = array(
    'title' => t('Create Project'),
    'description' => t('Clones the repo, and creates the platforms.'),
    'provision_save' => TRUE,
  );
  $tasks['project']['devshop-install'] = array(
    'title' => t('Install Sites'),
    'description' => t('Runs drupal installation on each of the three platforms.'),
  );
  $tasks['project']['devshop-platform-create'] = array(
    'title' => t('Create Platform'),
    'description' => t('Create a new platform to add to an existin project.'),
  );

  return $tasks;
}

/**
 * Implements hook_hosting_project_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For project entities.
 */
function devshop_projects_hosting_project_context_options(&$task) {
  $task->context_options['server'] = '@server_master';
  $task->context_options['project_name'] = $task->ref->title;
  $task->context_options['install_profile'] = $task->ref->install_profile;
  $task->context_options['base_url'] = $task->ref->base_url;
  $task->context_options['code_path'] = trim($task->ref->code_path, " ");
  $task->context_options['git_url'] = $task->ref->git_url;
}

/**
 * Implements hook_hosting_site_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_site_context_options(&$task) {
  $task->context_options['project'] = $task->ref->git_url;
}

/**
 * Implements hook_hosting_site_context_options()
 * 
 * This transfers data from the node to the aegir context object (the alias!)
 * For site entities.
 */
function devshop_projects_hosting_platform_context_options(&$task) {
  $task->context_options['project'] = $task->ref->project;
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
  //print_r($tasklist);
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

  $tasklist['devshop-platform-create']['nid'] = $node->nid;
  $tasklist['devshop-platform-create']['task_status'] = 1;
  $tasklist['devshop-platform-create']['exists'] = 1;



  // Override some
  unset($tasklist['devshop-create']['task_permitted']);
  unset($tasklist['devshop-create']['nid']);
  
  foreach ($tasklist as $task => $info) {
    $row = array();

    if (!isset($info['nid']) && !$info['task_permitted'] &&
        $task != 'devshop-platform-create') {
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

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Commit" task.
 */
function hosting_task_devshop_platform_create_form($project_node) {
  $form = array();
    
  //Bail if no platforms yet.
  if (!isset($project_node->project_objects['platform'])){
    $form['note'] = array(
      '#type' => 'item',
      '#title' => t('Clone & Verify'),
      '#value' => t('Your code is on its way to the server.  Once the platforms verify, you can choose an installation profile.'),
      );
    return $form;
  }

  $form['platform_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Platform Name'),
    '#required' => TRUE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => '',
    '#maxlength' => 255,
  );
  $form['git_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Git URL'),
    '#required' => FALSE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => '',
    '#maxlength' => 255,
  );
  $form['branch'] = array(
    '#type' => 'textfield',
    '#title' => t('Branch'),

    '#required' => TRUE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => 'master',
    '#maxlength' => 255,
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,  
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create platform'),
  );
  return $form;
}

