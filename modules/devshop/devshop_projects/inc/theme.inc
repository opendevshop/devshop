<?php

/**
 * Implements hook_hosting_theme()
 */
function devshop_projects_theme(){
  return array(
    'devshop_projects_settings_form' => array(
      'arguments' => array(
        'form' => NULL,
      ),
    ),
    'devshop_projects_create_settings_form' => array(
      'arguments' => array(
        'form' => NULL,
      ),
    ),
    'devshop_project_nav' => array(
      'arguments' => array(
        'node' => NULL,
      ),
      'template' => 'project-nav',
    ),
  );
}

/**
 * Theme function for environments settings
 */
function theme_devshop_projects_settings_form($form) {
  $rows = array();
  $header = array();
  $header[] = t('Environment');
  foreach (element_children($form) as $env_name) {
    $row = array();
    $row[] = $env_name . drupal_render($form[$env_name]['git_ref']);
    foreach(element_children($form[$env_name]['settings']) as $setting){
      if (!isset($header[$setting])){
        $header[$setting] = $form[$env_name]['settings'][$setting]['#title'];
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name]['settings'][$setting]);
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('id' => 'project-settings-table'));
  return $output;
}

/**
 * Theme function for create environments settings
 * @TODO: Fold into theme_devshop_projects_settings_form()
 */
function theme_devshop_projects_create_settings_form($form) {
  $rows = array();
  $header = array();
  foreach (element_children($form) as $env_name) {
    $row = array();
    $header['name'] = 'Name';
    $header['git_ref'] = t('Branch/Tag');
    $row[] = drupal_render($form[$env_name]['name']);
    $row[] = drupal_render($form[$env_name]['git_ref']);

    foreach(element_children($form[$env_name]['settings']) as $setting){
      if (!isset($header[$setting])){
        $header[$setting] = $form[$env_name]['settings'][$setting]['#title'];
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name]['settings'][$setting]);
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('id' => 'project-settings-table', 'class' => 'table'));
  $output .= '<p>'. t('Create as many new environments as you would like. For example: "dev", "test", and "live". You can create more later on if needed.') .'</p>';

  return $output; 
}

/**
 * Preprocess page
 */
function devshop_projects_preprocess_page(&$vars){

  // On project node edit page
  if (isset($vars['node']) && $vars['node']->type == 'project' && arg(2) == 'edit'){

    $links = array();
    $node = node_load($vars['node']->nid);
    foreach ($node->project->environments as $environment){
      $links[] = l($environment->name, 'node/' . $node->nid . '/edit/' . $environment->name);
    }
    $vars['tabs2'] = theme('item_list', $links, '', 'ul', array('class' => 'tabs secondary'));
  }
}


/**
 * Preprocess page
 */
function devshop_projects_preprocess_node(&$vars){

  // On project node edit page
  if (isset($vars['node']) && $vars['node']->type == 'project'){

    $vars['drush_aliases'] = devshop_project_aliases($vars['node']->project);
    $vars['aliases_url'] = url("node/{$vars['node']->nid}/aliases");

    if (module_exists('aegir_ssh') && user_access('manage own SSH public keys')) {
      global $user;
      $vars['access_note'] = t('NOTE: To access these environments with drush remotely, make sure you have uploaded your public SSH key under !link.', array(
        '!link' => l('My Account > SSH Keys', "user/$user->uid/ssh-keys"),
      ));
    }
    else {
      $vars['access_note'] = t('NOTE: To access these environments with drush remotely, ask an administrator to add your public SSH key to the file <code>/var/aegir/.ssh/authorized_keys</code>.');
    }
  }
}



/**
 * Preprocess for project_nav.tpl.php
 */
function template_preprocess_devshop_project_nav(&$vars){

  global $user;
  $node = $vars['node'];
  $project = $vars['project'] = $vars['node']->project;

  // @TODO: Detect other web URLs for other git hosts.
  if ($project->git_repo_url) {
    $vars['github_url'] = $project->git_repo_url;
  }

  // Generate branches/tags lists
  $vars['branches_count'] = count($project->settings->git['branches']);
  $vars['tags_count'] = count($project->settings->git['tags']);
  $vars['branches_items'] = array();
  $vars['branches_icon'] = 'code-fork';

  if ($vars['branches_count'] == 0){
    // If branches are 0 and last verifying is queued...
    if ($node->verify->task_status == HOSTING_TASK_PROCESSING || $node->verify->task_status == HOSTING_TASK_QUEUED) {
      $vars['branches_show_label'] = TRUE;
      $vars['branches_label'] = t('Refreshing...');
      $vars['branches_class'] = 'btn-warning';
      $vars['branches_icon'] = 'gear fa-spin';
      $vars['branches_items'][] = l(t('View task log'), 'node/' . $node->verify->nid);

    }
    // If branches are 0 and last verifying failed...
    elseif ($node->verify->task_status == HOSTING_TASK_ERROR) {
      $vars['branches_show_label'] = TRUE;
      $vars['branches_label'] = t('Error');
      $vars['branches_class'] = 'btn-danger';
      $vars['branches_items'][] = t('There was a problem refreshing branches and tags.');
      $vars['branches_items'][] = l(t('View task log'), 'node/' . $node->verify->nid);
      $vars['branches_items'][] = l(t('Refresh branches'), 'node/' . $node->nid . '/project_verify', array('attributes' => array('class' => 'refresh-link'), 'query' => array('token' => drupal_get_token($user->uid))));
    }
    // If branches are 0 and last verifying has completed... This should never happen, because the task would error out.
    elseif ($node->verify->task_status == HOSTING_TASK_SUCCESS) {
      $vars['branches_show_label'] = TRUE;
      $vars['branches_label'] = t('No branches found!');
      $vars['branches_items'][] = l(t('Refresh branches'), 'node/' . $node->nid . '/project_verify', array('attributes' => array('class' => 'refresh-link'), 'query' => array('token' => drupal_get_token($user->uid))));
    }
  }
  // If there are branches... build the branch items
  else {
    $vars['branches_show_label'] = FALSE;
    $vars['branches_label'] = format_plural($vars['branches_count'], t('1 Branch'), t('!count Branches', array('!count' => $vars['branches_count'])));

    foreach ($project->settings->git['branches'] as $branch){
      $href = isset($vars['github_url'])? $vars['github_url'] . '/tree/' . $branch: '#';
      $vars['branches_items'][] = "<a href='$href'><i class='fa fa-code-fork'></i> $branch </a>";
    }
  }

  if ($vars['tags_count']){
//      <li class="divider"></li>

    $vars['branches_label'] .= ' &amp; ' . format_plural($vars['tags_count'], t('1 Tag'), t('!count Tags', array('!count' => $vars['tags_count'])));


    foreach ($project->settings->git['tags'] as $branch){
      $href = isset($vars['github_url'])? $vars['github_url'] . '/tree/' . $branch: '#';
      $vars['branches_items'][] = "<a href='$href'><i class='fa fa-tag'></i> $branch </a>";
      $vars['git_refs'][] = $branch;
    }
  }

  $vars['dashboard_link'] = l('<i class="fa fa-cubes"></i> ' . t('Dashboard'), "node/$project->nid", array('html' => TRUE));

  if (node_access('update', $node)){
    $vars['settings_link'] = l('<i class="fa fa-sliders"></i> ' . t('Settings'), "node/$node->nid/edit", array('html' => TRUE));
  }

  if (arg(2) == 'edit') {
    $vars['settings_active'] = 'active';
  }
  if (arg(2) == 'logs') {
    $vars['logs_active'] = 'active';
  }
}