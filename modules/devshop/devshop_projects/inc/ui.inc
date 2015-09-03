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
    'Git URL',
    'Environments',
  );

  $query = db_query("SELECT n.nid FROM {hosting_devshop_project} p LEFT JOIN {node} n ON n.nid = p.nid WHERE n.status = 1 ORDER BY n.changed");
  $rows = array();

  while ($result = db_fetch_object($query)) {
    $project_node = node_load($result->nid);
    $project = $project_node->project;

    // Load an environment for extra info like version.
    // @TODO: load the chosen live environment.
    $environment = array_shift(array_values($project->environments));
    $platform_node = node_load($environment->platform);

    $row = array();

    // Link to Project page
    $row[] = '<strong>' . l($project->name, "node/$project->nid") . '</strong>';

    // Install Profile
    $row[] = $project->install_profile;

    // Drupal Version
    $row[] = $platform_node->release->version;

    // Git URL
    $row[] = strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => $project->git_url));

    // Links to all sites
    $actions = array();
    $count = 0;
    foreach ($project->environments as $env => $details){
      // @TODO: Handle aliases.
      $site_url = url("http://$env.$project->base_url", array('absolute' => TRUE));
      $actions[] = array(
        'title' => $site_url,
        'href' => $site_url,
        'attributes' => array('target' => '_blank'),
      );
      $count++;
    }
    $label = format_plural($count, t('1 Environment'), t('@num Environments', array('@num' => $count)));
    $row[] = theme('ctools_dropdown', $label, $actions);

    $rows[] = $row;
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

    $output = theme('table', $data['header'], $data['rows'], array('class' => 'table'));
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
  $project = $node->project;

  // Check to see if this project is still in the wizard
  ctools_include('object-cache');
  $project_wizard_cache = ctools_object_cache_get('project', NULL);
  if (isset($project_wizard_cache->project_nid) && $node->nid == $project_wizard_cache->project_nid){
    drupal_goto('hosting/projects/add/' . $project_wizard_cache->step);
  }

  $node->content['info'] = array(
    '#prefix' => '<div id="hosting-platform-info">',
    '#suffix' => '</div>'
  );

  $node->content['info'] = array(
    '#value' => devshop_project_aliases($node->project),
  );

  //$node->content['info']['code_path'] = array(
  //  '#type' => 'item',
  //  '#title' => t('Code path'),
  //  '#value' => filter_xss($node->code_path),
  //  '#weight' => -8
  //);

  if (!empty($project->settings->live['live_domain'])) {
    $url = 'http://' . $project->settings->live['live_domain'];

    $node->content['info']['live_domain'] = array(
      '#type' => 'item',
      '#title' => t('Live Site'),
      '#value' => l($url, $url, array('attributes' => array('target' => '_blank'))),
      '#weight' => -9,
    );
  }

  $node->content['info']['git_url'] = array(
    '#type' => 'item',
    '#title' => t('Git Repository'),
    '#value' => strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => $project->git_url)),
    '#weight' => -10
  );

  // Detect GitHub
  // @TODO: Detect other web URLs for other git hosts.
  if (strpos($project->git_url, 'github.com') !== FALSE) {
    $url = str_replace('git@github.com:', 'http://github.com/', $project->git_url);
    $node->content['info']['git_url']['#value'] .= l(t('View on GitHub.com'), $url, array('attributes' => array('target' => '_blank')));
  }

  // Branches display
  if (!empty($project->settings->git['branches'])){
    $items = theme_item_list($project->settings->git['branches'], NULL, 'ul', array('class' => 'branches'));
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

  // Install profile display
  if (!empty($project->install_profile)){
    $node->content['info']['install_profile'] = array(
      '#type' => 'item',
      '#title' => t('Install profile'),
      '#value' => ($project->install_profile),
      '#weight' => -7
    );
  }

  // Environments
  $rows = array();

  foreach ($project->environments as $env => $environment) {
    $site_installed = false;

    // Skip this if it is not enabled.
    if ($environment->site_status == HOSTING_SITE_DELETED && $environment->platform_status == HOSTING_PLATFORM_DELETED) {
      continue;
    }

    // @TODO: Remove. Only devshop_hosting_site_goto_link() requires site node.
    $site = node_load($environment->site);

    $row = array();
    $row[] = "<span class='environment'>$env</span>";
    if ($environment->site_status == HOSTING_SITE_DISABLED){
      $output = '<em>' . t('Disabled') . '</em>';
      $site_installed = TRUE;

      // A "disabled" site might be a failed install. Load and display.
      $task = hosting_get_most_recent_task($environment->site, 'install');
      if ($task->task_status == HOSTING_TASK_ERROR) {
        $output = t('Site install failed.');
        $output .= ' ' . l(t('View the logs and retry'), "node/$task->nid") . '.';
      }
      elseif ($task->task_status == HOSTING_TASK_PROCESSING  || $task->task_status == HOSTING_TASK_QUEUED) {
        $output = '<em>' . t('Installing...') . '</em>';
      }

      $row[] = $output;

    } elseif ($environment->site_status == HOSTING_SITE_QUEUED || $environment->site_status == HOSTING_SITE_PROCESSING ){
      $row[] = '<em>' . t('Installing...') . '</em>';
    } else {
      $row[] = devshop_hosting_site_goto_link($site);
      $site_installed = TRUE;
    }

    // Create branch/tag chooser
    $actions = array();
    if (!empty($project->settings->git['branches'])) {
      foreach ($project->settings->git['branches'] as $branch){
        $actions[] = array(
          'title' => '<i class="fa fa-code-fork"></i> ' . $branch,
          'href' => 'node/' . $node->nid . '/project_devshop-deploy/ref/' . $branch,
          'query' => array(
            'token' => drupal_get_token($user->uid),
            'environment' => $environment->name,
          ),
          'html' => TRUE,
          'attributes' => array(
            'class' => 'hosting-button-dialog git-branch',
            'title' => t('Deploy branch %ref to %environment environment', array('%ref' => $branch, '%environment' => $environment->name)),
          ),
        );

        // Detect current ref
        if ($environment->git_ref == $branch){
          $current_ref = 'Branch';
        }
      }
    }
    else {
      $actions[] = array(
        'title' => t('No branches found.  Click to Refresh.'),
        'href' => 'node/' . $node->nid . '/project_verify',
        'query' => array(
          'token' => drupal_get_token($user->uid),
        ),
      );
    }

    if (!empty($project->settings->git['tags'])) {
      foreach ($project->settings->git['tags'] as $tag){
        $actions[] = array(
          'title' => '<i class="fa fa-tag"></i> ' . $tag,
          'href' => 'node/' . $node->nid . '/project_devshop-deploy/ref/' . $tag,
          'query' => array(
            'token' => drupal_get_token($user->uid),
            'environment' => $environment->name,
          ),
          'html' => TRUE,
          'attributes' => array(
            'class' => 'hosting-button-dialog git-tag',
            'title' => t('Deploy tag %ref to %environment environment', array('ref' => $tag)),
          ),
        );

        // Detect current ref
        if ($environment->git_ref == $tag){
          $current_ref = 'Tag';
        }
      }
    }

    $row[] = theme('ctools_dropdown', $current_ref . ': ' . $environment->git_ref, $actions);

    if (module_exists('devshop_log')) {
      $row[] =l(t('Commits'), "node/$environment->site/logs/commits");
    }
    if (module_exists('hosting_logs')) {
      $row[] = l(t('Errors'), "node/$environment->site/logs/errors");
    }
    if (module_exists('hosting_filemanager')) {
      $row[] = l(t('Files'), "node/$environment->site/files/platform");
    }
    
    // Create actions dropdown.
    $actions = array();
    // Aegir Tasks
    if ($site_installed){
      $actions[] = array(
        'title' => t('Clone environment'),
        'href' => 'node/' . $node->nid . '/project_devshop-create/clone/' . $environment->site, array( 'query' => array('token' => drupal_get_token($user->uid))),
        'attributes' => array(
          'class' => 'hosting-button-dialog',
        ),
      );
      $actions[] = array(
        'title' => t('Fork environment'),
        'href' => 'node/' . $node->nid . '/project_devshop-create/fork/' . $environment->site, array( 'query' => array('token' => drupal_get_token($user->uid))),
        'attributes' => array(
          'class' => 'hosting-button-dialog',
        ),
      );

      $site_tasklist = hosting_task_fetch_tasks($environment->site);
      $site_tasklist['restore']['title'] = t('Restore Backups');
    }
    else {
      $site_tasklist = array();
      $actions[] = array(
        'title' => t('Delete Platform'),
        'href' => 'node/' . $environment->platform . '/platform_delete/', array( 'query' => array('token' => drupal_get_token($user->uid))),
        'attributes' => array(
          'class' => 'hosting-button-dialog',
        ),
      );
    }

    // Invoke hook_devshop_environment_actions()
    // Load available environment actions.
    $environment_actions = module_invoke_all('devshop_environment_actions', $environment, $site_tasklist);

    // @TODO: Invoke drupal_alter()

    // Build links to tasks
    foreach ($environment_actions as $task_name) {
      if (isset($site_tasklist[$task_name]) && $site_tasklist[$task_name]['task_permitted']){
        $actions[] = array(
          'title' => $site_tasklist[$task_name]['title'],
          'href' => sprintf("node/%d/%s_%s", $environment->site, 'site', $task_name),
          'query' => array(
            'token' => drupal_get_token($user->uid),
            'destination' => "node/$node->nid",
          ),
          'attributes' => array(
            'class' => isset($site_tasklist[$task_name]['dialog'])? 'hosting-button-dialog': '',
          ),
        );
      }
    }

    // Generate URLs for all actions.
    foreach ($actions as &$action) {
      $action['url'] = url($action['href'], $action);
    }

    $node->environment_actions[$environment->name] = $actions;
    
    $row[] = theme('ctools_dropdown', t('Actions'), $actions);
    $rows[] = $row;
//
//    if (!is_array($environment->tasks)) $environment->tasks = array();
//    foreach ($environment->tasks as &$task) {
//      if ($task->task_status == HOSTING_TASK_QUEUED || $task->task_status == HOSTING_TASK_PROCESSING){
//        $environment->active_tasks++;
//      }
//    }
//
//    // Progress Bar
//    if ($environment->active_tasks > 0){
//      $environment->progress_classes = 'progress-bar-striped progress-bar-warning active';
//      $environment->active_tasks++;
//    } else {
//      $environment->progress_classes = 'inactive';
//    }
  }
  $header = array();
  $table = theme('table', $header, $rows, array('class' => 'environments-table'));

  $node->content['environments'] = array(
    '#type' => 'fieldset',
    '#title' => t('Environments'),
    '#weight' => 12,
    '#attributes' => array(
      'class' => 'project-environments',
    ),
  );

  $link = l(t('Create New Environment'), 'node/' . $node->nid . '/project_devshop-create', array('attributes' => array('class' => 'create-new-environment hosting-button-dialog'), 'query' => array('token' => drupal_get_token($user->uid))));
  $node->content['environments']['table'] = array(
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
 * Implements hook_nodeapi_TYPE_OP() for site nodes and view op.
 */
function devshop_projects_nodeapi_site_view(&$node, $a3, $a4) {
  if (!empty($node->project)){
    // Display Project, Environment and Branch.
    $node->content['info']['project'] = array(
      '#type' => 'item',
      '#title' => t('Project'),
      '#value' => l($node->project->name, "node/{$node->project->nid}"),
      '#weight' => -12,
    );
    $node->content['info']['env'] = array(
      '#type' => 'item',
      '#title' => t('Environment'),
      '#value' => $node->environment->name,
      '#weight' => -11,
    );
    $node->content['info']['branch'] = array(
      '#type' => 'item',
      '#title' => t('Branch/Tag'),
      '#value' => $node->environment->git_ref,
      '#weight' => -11,
    );
    
    // Add Breadcrumbs
    $crumbs = array();
    $crumbs[] = l(t('Home'), '<front>');
    $crumbs[] = l(t('Projects'), 'hosting/projects');
    $crumbs[] = l($node->project->name, "node/" . $node->project->nid);
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
 * Implements hook_nodeapi_TYPE_OP() for site nodes and view op.
 */
function devshop_projects_nodeapi_task_view(&$node, $a3, $a4) {

  $rows = array();
  foreach ($node->task_args as $key => $value) {
    $rows[] = array(
      '<strong>' . $key . '</strong>',
      '<small>' . $value . '</small>',
    );
  }

  $task_host = node_load($node->rid);

  $node->content['info']['project'] = array(
    '#type' => 'item',
    '#title' => t('Project'),
    '#value' => l($task_host->project->name, "node/{$task_host->project->nid}"),
    '#weight' => -12,
  );
  $node->content['info']['parameters'] = array(
    '#type' => 'item',
    '#title' => t('Task Parameters'),
    '#value' => theme('table', array(),  $rows),
    '#weight' => -11,
  );

}

/**
 * Our own version of _hosting_site_goto_link()
 */
function devshop_hosting_site_goto_link($node) {
//  $project = node_load($node->environment->project_nid);

  // If this is the live environment, use the live domain.
  if ($node->environment->name == $node->project->settings->live['live_environment']) {
    $url = $node->project->settings->live['live_domain'];
  }
  // If environment aliases are enabled
  elseif ($node->project->settings->live['environment_aliases']) {
    $url = "{$node->environment->name}.{$node->project->settings->live['live_domain']}";
  }
  // Otherwise use the base_url
  else {
    $url = "{$node->environment->name}.{$node->project->base_url}";
  }

  // Get the link from cache.
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
