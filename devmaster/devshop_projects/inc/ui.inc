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
    'Environments',
  );

  $r = db_query("SELECT hdp.* FROM {hosting_devshop_project} hdp LEFT JOIN {node} n ON hdp.nid = n.nid WHERE n.status = 1 ORDER BY n.title");
  $rows = array();

  while(($proj = db_fetch_object($r))) {
    $node = node_load($proj->nid);
    if (!empty($node->project_objects['platform'])){
      $platform_node = node_load(key($node->project_objects['platform']));
    }

    $row = array();

    // Link to Project page
    $row[] = '<strong>' . l($node->title, "node/$proj->nid") . '</strong>';

    // Install Profile
    $row[] = $node->install_profile;

    // Drupal Version
    $row[] = $platform_node->release->version;

    // Number of environments
    $num = db_result(db_query("SELECT COUNT(*) FROM {hosting_devshop_project_object} hdpo LEFT JOIN {hosting_site} hs ON hdpo.object_nid = hs.nid WHERE hdpo.project_nid = %d AND hdpo.object_type='site' AND hs.status=1", $node->nid));
    $row[] = format_plural($num, t('1 site'), t('!num sites', array('!num' => $num)));

    // Git URL
    $row[] = strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => $node->git_url));

    // Links to all sites
    $actions = array();
    $count = 0;
    foreach ($node->project->environments as $env => $details){
      $dev_site_url = url("http://$env.$node->base_url", array('absolute' => TRUE));
      $actions[] = array(
        'title' => $dev_site_url,
        'href' => $dev_site_url,
        'attributes' => array('target' => '_blank'),
      );
      $count++;
    }
    $label = format_plural($count, t('1 Environment'), t('@num Environments', array('@num' => $count)));
    $row[] = theme('ctools_dropdown', $label, $actions);

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
  global $user;

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

  if (!empty($node->live_domain)) {
    $url = 'http://' . $node->live_domain;

    $node->content['info']['live_domain'] = array(
      '#type' => 'item',
      '#title' => t('Live Site'),
      '#value' => l($url, $url, array('attributes' => array('target' => '_blank'))),
      '#weight' => -9,
    );
  }

  $node->content['info']['git_url'] = array(
    '#type' => 'item',
    '#title' => t('Git URL'),
    '#value' => strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => $node->git_url)),
    '#weight' => -10
  );

  // Branches display
  if (!empty($node->project->git['branches'])){
    $items = theme_item_list($node->project->git['branches'], NULL, 'ul', array('class' => 'branches'));
  }
  else {
    $items = theme_item_list(array('No Branches Found!'), NULL, 'ul', array('class' => 'branches'));
  }
  $verify_task = hosting_get_most_recent_task($node->nid, 'verify');

  if ($verify_task->task_status == HOSTING_TASK_SUCCESS || $verify_task->task_status == HOSTING_TASK_ERROR){
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

  if (!empty($node->install_profile)){
    $node->content['info']['install_profile'] = array(
      '#type' => 'item',
      '#title' => t('Install profile'),
      '#value' => ($node->install_profile),
      '#weight' => -7
    );
  }

  // Environments
  $rows = array();
  foreach ($node->project->environments as $env => $environment) {
    
    $site_nid = $environment['site'];
    $platform_nid = $environment['platform'];

    $site = node_load($site_nid);
    $platform = node_load($platform_nid);
    
    // Skip this if it is not enabled.
    if ($site->site_status == HOSTING_SITE_DELETED && $platform->platform_status == HOSTING_PLATFORM_DELETED) {
      continue;
    }

    $row = array();
    $row[] = "<span class='environment'>$env</span>";
    if ($site->site_status == HOSTING_SITE_DISABLED){
      $row[] = devshop_hosting_site_goto_link($site) . '<em>' . t('Disabled') . '</em>';
    } elseif ($site->site_status == HOSTING_SITE_QUEUED){
      $row[] = '<em>' . t('Installing...') . '</em>';
    } else {
      $row[] = devshop_hosting_site_goto_link($site);
    }

    // Display current branch
    $row[] = "<span class='branch environment-branch' title='Branch: $site->git_branch'>$site->git_branch</span>";

    // Create branch/tag chooser
    // @TODO: Uncomment once we have a link to go to.
//    $actions = array();
//    foreach ($node->project->git['branches'] as $branch){
//      $actions[] = array(
//        'title' => $branch,
//        'href' => "node/" . $platform_nid . "/deploy",
//      );
//    }
//    foreach ($node->project->git['tags'] as $tag){
//      $actions[] = array(
//        'title' => "tag/" . $tag,
//        'href' => "node/" . $platform_nid . "/deploy",
//      );
//    }
//    $row[] = theme('ctools_dropdown', $site->git_branch, $actions);

    if (module_exists('devshop_log')) {
      $row[] =l(t('Commits'), "node/$site->nid/logs/commits");
    }
    if (module_exists('hosting_logs')) {
      $row[] = l(t('Errors'), "node/$site->nid/logs/errors");
    }
    if (module_exists('hosting_filemanager')) {
      $row[] = l(t('Files'), "node/$site->nid/files/platform");
    }
    
    // Create actions dropdown.
    $actions = array();
    $actions[] = array(
      'title' => t('Fork environment'),
      'href' => 'node/' . $node->nid . '/project_devshop-create/' . $site->nid, array( 'query' => array('token' => drupal_get_token($user->uid))),
      'attributes' => array(
        'class' => 'hosting-button-dialog',
      ),
    );
    
    
    // Aegir Tasks
    $site_tasklist = hosting_task_fetch_tasks($site->nid);
    $site_tasklist['restore']['title'] = t('Restore Backups');

    // The actions we want
    $site_actions = array('flush_cache', 'login_reset', 'backup', 'restore');

    // Add disable or delete task based on hosting variable.
    if (!variable_get('hosting_require_disable_before_delete', TRUE)){
      $site_actions[] = 'delete';
    } else {
      if ($site->site_status == HOSTING_SITE_DISABLED){
        $site_actions[] = 'enable';
        $site_actions[] = 'delete';
      } else {
        $site_actions[] = 'disable';
      }
    }
    
    // Build links to tasks
    foreach ($site_actions as $task_name) {
      if ($site_tasklist[$task_name]['task_permitted']){
        $actions[] = array(
          'title' => $site_tasklist[$task_name]['title'],
          'href' => sprintf("node/%d/%s_%s", $site->nid, $site->type, $task_name),
          'query' => array(
            'token' => drupal_get_token($user->uid),
            'destination' => "node/$node->nid",
          ),
          'attributes' => array(
            'class' => $site_tasklist[$task_name]['dialog']? 'hosting-button-dialog': '',
          ),
        );
      }
    }
    
    // Grant access to Aegir pages
    if (user_access('administer hosting')){
      $actions[] = array(
        'title' =>  t('Site Dashboard'),
        'href' => "node/$site->nid",
      );
      $actions[] = array(
        'title' =>  t('Platform Dashboard'),
        'href' => "node/$site->platform",
      );
    }
    
    $row[] = theme('ctools_dropdown', t('Actions'), $actions);
    $rows[] = $row;
  }
  $header = array();
  $table = theme('table', $header, $rows, array('class' => 'environments-table'));

  $node->content['sites'] = array(
    '#type' => 'fieldset',
    '#title' => t('Environments'),
    '#weight' => 12,
    '#attributes' => array(
      'class' => 'project-environments',
    ),
  );

  $link = l(t('Create New Environment'), 'node/' . $node->nid . '/project_devshop-create', array('attributes' => array('class' => 'create-new-environment hosting-button-dialog'), 'query' => array('token' => drupal_get_token($user->uid))));
  $node->content['sites']['table'] = array(
    '#type' => 'item',
    '#value' => $table,
    '#suffix' => $link,
  );

  //Tasks
  $tasks_table = hosting_task_table($node);
  $node->content['tasks_view'] = array(
    '#type' => 'item',
    '#value' => $tasks_table,
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
  $project = node_load($node->project_nid);
  $url = "{$node->environment}.{$project->base_url}";
  $cache = cache_get("hosting:site:" . $node->nid . ":login_link");
  if (!is_null($cache) && (time() < $cache->data['expire'])) {
    $title = t("Log in: !url", array('!url' => $url));
  }
  else {
    $title = t("!url", array('!url' => $url));
  }
  $options['attributes']['target'] = '_blank';
  $options['attributes']['class'] = 'hosting-goto-site-link';
  return l($title, "node/" . $node->nid . "/goto_site", $options);
}
