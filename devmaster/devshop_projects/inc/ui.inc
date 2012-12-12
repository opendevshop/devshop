<?php

/**
 * Projects Page
 *
 * Callback that is invoked when the user points the brower to
 * hosting/projects. It displays a nice tabulated list of projects
 * and cool things you can do with them.
 *
 * @TODO: Should we make this a list of teasers?  Hard to cram everything in
 * a table.
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

/**
 * Implementation of hook_view().
 *
 * Project Page Display
 */
function devshop_projects_view($node, $teaser = FALSE, $page = FALSE) {

  // If not ready yet:
  if ($node->project_status != 'sites_ready'){
    // Site install form
    $node->content['devshop']['install'] = array(
      '#value' => drupal_get_form('devshop_projects_install_sites_form', $node),
      '#type' => 'markup',
    );
    return $node;
  }

  modalframe_parent_js();

  $node->content['info'] = array(
    '#prefix' => '<div id="hosting-platform-info">',
    '#suffix' => '</div>'
  );

  $node->content['info']['code_path'] = array(
    '#type' => 'item',
    '#title' => t('Code path'),
    '#value' => filter_xss($node->code_path),
    '#weight' => -8
  );

  $url = 'http://dev.' . $node->base_url;
  $node->content['info']['base_url'] = array(
    '#type' => 'item',
    '#title' => t('Dev Site'),
    '#value' => l($url, $url, array('attributes' => array('target' => '_blank'))),
    '#weight' => -10
  );

  $node->content['info']['git_url'] = array(
    '#type' => 'item',
    '#title' => t('Git URL'),
    '#value' => strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => $node->git_url)),
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
        $row[] = l('Site Dashboard', "node/$site->nid");
      }
      else {
        // Site if deleted, just show the platform name
        $row[] = $platform->title;
        $row[] = '';
      }
      $row[] = l('Platform Dashboard', "node/$site->platform");

      if (module_exists('devshop_log')){
        $row[] = l('Commit Log', "node/$site->platform/gitlog");
      }
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
  return $node;
}
