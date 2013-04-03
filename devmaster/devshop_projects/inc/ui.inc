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
function devshop_projects_projects_page() {
  drupal_set_title(t('Projects'));
  $header = array(
    'Name',
    'Profile',
    'Version',
    '',
    'Git URL',
    'Dev Site',
  );

  $r = db_query("SELECT hdp.* FROM {hosting_devshop_project} hdp LEFT JOIN {node} n ON hdp.nid = n.nid WHERE n.status = 1 ORDER BY n.title");
  $rows = array();

  while(($proj = db_fetch_object($r))) {
    $node = node_load($proj->nid);
    if (!empty($node->project_objects['platform'])){
      $platform_node = node_load(key($node->project_objects['platform']));
    }

    $row = array();
    $link_options = array('attributes' => array('target' => '_blank'));

    // Link to Project page
    $row[] = '<strong>' . l($node->title, "node/$proj->nid") . '</strong>';

    // Install Profile
    $row[] = $node->install_profile;

    // Drupal Version
    $row[] = $platform_node->release->version;

    // Number of environments
    $num = db_result(db_query("SELECT COUNT(*) FROM {hosting_devshop_project_object} WHERE project_nid = %d AND object_type='site'", $node->nid));
    $row[] = format_plural($num, t('1 site'), t('!num sites', array('!num' => $num)));



    // Git URL
    $row[] = strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => $node->git_url));

    // Link to Dev Site
    $dev_site_url = url("http://dev." . $node->base_url, array('absolute' => TRUE));
    $row[] = l($dev_site_url, $dev_site_url, $link_options);


    $rows[$proj->nid] = $row;
  }

  // No Projects
  if (empty($rows)){
    $button = l(t('Create your first Project'), 'hosting/projects/add');
    $output = <<<HTML
<div class="empty">
    <div class="empty-message">You have no projects.</div>
    <div class="empty-button">$button</div>
</div>
HTML;
  } else {
    //Call hooks for alter informacion
    $data = module_invoke_all('devshop_projects_page', $rows, $header);
    
    // @TODO: Better hook...
    if (empty($data['rows'])){
      $data['rows'] = $rows;
    }
    if (empty($data['header'])){
      $data['header'] = $header;
    }
  
    $output = theme('table', $data['header'], $data['rows'], array('class' => 'hosting-table'));  
  }
  return $output;
}

/**
 * Implementation of hook_view().
 *
 * Project Page Display
 */
function devshop_projects_view($node, $teaser = FALSE, $page = FALSE) {

  // Check to see if this project is still in the wizard
  ctools_include('object-cache');
  $project_wizard_cache = ctools_object_cache_get('project', NULL);
  if ($node->nid == $project_wizard_cache->project_nid){
    drupal_goto('hosting/projects/add/' . $project_wizard_cache->step);
  }

  modalframe_parent_js();

  $node->content['info'] = array(
    '#prefix' => '<div id="hosting-platform-info">',
    '#suffix' => '</div>'
  );

  //$node->content['info']['code_path'] = array(
  //  '#type' => 'item',
  //  '#title' => t('Code path'),
  //  '#value' => filter_xss($node->code_path),
  //  '#weight' => -8
  //);
  //
  //$url = 'http://dev.' . $node->base_url;
  //$node->content['info']['base_url'] = array(
  //  '#type' => 'item',
  //  '#title' => t('Dev Site'),
  //  '#value' => l($url, $url, array('attributes' => array('target' => '_blank'))),
  //  '#weight' => -10
  //);

  $node->content['info']['git_url'] = array(
    '#type' => 'item',
    '#title' => t('Git URL'),
    '#value' => strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => $node->git_url)),
    '#weight' => -10
  );

  if (!empty($node->git_branches)){
   $items = theme_item_list($node->git_branches, NULL, 'ul', array('class' => 'branches')); 
    $verify_task = hosting_get_most_recent_task($node->nid, 'verify');
    
    if ($verify_task->task_status == HOSTING_TASK_SUCCESS){    
      $refresh_link =  l(t('Refresh branches'), 'node/' . $node->nid . '/project_verify', array('attributes' => array('class' => 'refresh-link'), 'query' => array('token' => drupal_get_token($user->uid))));
    } else {
      $refresh_link =  t('Refresh in progress...');
    }
    
    $node->content['info']['git_branches'] = array(
      '#type' => 'item',
      '#title' => t('Remote Branches'),
      '#value' => '<div class="branches-list">' . $items . $refresh_link . '</div>',
      '#weight' => -8,
    );
  }

  if (!empty($node->install_profile)){
    $node->content['info']['install_profile'] = array(
      '#type' => 'item',
      '#title' => t('Install profile'),
      '#value' => ($node->install_profile),
      '#weight' => -7
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
        $branch = "<span class='branch'>$site->git_branch</span>";
        $row[] = devshop_hosting_site_goto_link($site) . $branch;
        $row[] = l('Site Dashboard', "node/$site->nid");
      }
      else {
        // Site if deleted, just show the platform name
        $row[] = $platform->title;
        $row[] = '';
      }
      $row[] = l('Platform Dashboard', "node/$site->platform");

      if (module_exists('devshop_log')){
        $row[] = l('Commit Log', "node/$site->nid/logs/commits");
      }
      if (module_exists('hosting_logs')){
        $row[] = l('Error Log', "node/$site->nid/logs/errors");
      }
      if (module_exists('hosting_filemanager')){
        $row[] = l('Files', "node/$site->nid/files/platform");
      }
      $rows[] = $row;
    }
    $header = array();
    $table = theme('table', $header, $rows);

    $node->content['sites'] = array(
      '#type' => 'fieldset',
      '#title' => t('Environments'),
      '#weight' => 12,
      '#attributes' => array(
        'class' => 'project-environments',
      ),
    );
    global $user;
    $link = l(t('Create New Environment'), 'node/' . $node->nid . '/project_devshop-create', array('attributes' => array('class' => 'create-new-environment'), 'query' => array('token' => drupal_get_token($user->uid))));
    $node->content['sites']['table'] = array(
      '#type' => 'item',
      '#value' => $table,
      '#suffix' => $link,
    );
  }

  //Tasks
  $node->content['tasks_view'] = array(
    '#type' => 'item',
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
  
  
  // Add Breadcrumbs
  $crumbs = array();
  $crumbs[] = l(t('Home'), '<front>');
  $crumbs[] = l(t('Projects'), 'hosting/projects');
  drupal_set_breadcrumb($crumbs);
  
  return $node;
}

/**
 * Implements hook_nodeapi_TYPE_OP()
 */
function devshop_projects_nodeapi_site_view(&$node, $a3, $a4) {
  if (!empty($node->project)){
    
    
    // Display Project, Environment and Branch.
    $node->content['info']['project'] = array(
      '#type' => 'item',
      '#title' => t('Project'),
      '#value' => l($node->project, "node/{$node->project_nid}"),
      '#weight' => -12,
    );
    $node->content['info']['env'] = array(
      '#type' => 'item',
      '#title' => t('Environment'),
      '#value' => $node->environment,
      '#weight' => -11,
    );
    $node->content['info']['branch'] = array(
      '#type' => 'item',
      '#title' => t('Branch'),
      '#value' => $node->git_branch,
      '#weight' => -11,
    );
    
    // Add Breadcrumbs
    $crumbs = array();
    $crumbs[] = l(t('Home'), '<front>');
    $crumbs[] = l(t('Projects'), 'hosting/projects');
    $crumbs[] = l($node->project, "node/" . $node->project_nid);
    drupal_set_breadcrumb($crumbs);
  }
}

/**
 * Implements hook_nodeapi_TYPE_OP()
 */
function devshop_projects_nodeapi_platform_view(&$node, $a3, $a4) {
  devshop_projects_nodeapi_site_view($node, $a3, $a4);
}

/**
 * Our own version of _hosting_site_goto_link()
 */
function devshop_hosting_site_goto_link($node) {
  $cache = cache_get("hosting:site:" . $node->nid . ":login_link");
  if (!is_null($cache) && (time() < $cache->data['expire'])) {
    $title = t("Log in: !url", array('!url' => $node->title));
  }
  else {
    $title = t("!url", array('!url' => $node->title));
  }
  $options['attributes']['target'] = '_blank';
  $options['attributes']['class'] = 'hosting-goto-site-link';
  return l($title, "node/" . $node->nid . "/goto_site", $options);
}