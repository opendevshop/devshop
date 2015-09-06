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
  $project = &$node->project;

  // Check to see if this project is still in the wizard
  ctools_include('object-cache');
  $project_wizard_cache = ctools_object_cache_get('project', NULL);
  if ($page && isset($project_wizard_cache->project_nid) && $node->nid == $project_wizard_cache->project_nid){
    drupal_goto('hosting/projects/add/' . $project_wizard_cache->step);
  }

  $tasks = hosting_available_tasks('site');

  // Load environment tasks
  foreach ($project->environments as &$environment) {
    $environment->tasks = devshop_get_tasks($environment);
    $environment->task_count = count($environment->tasks);
    $environment->active_tasks = 0;

    $items = array();
    $items[] = l('<i class="fa fa-list"></i> ' . t('Task Logs'), "node/$project->nid/logs/$environment->name", array(
      'html' => TRUE,
        'attributes' => array(
          'class' => 'list-group-item',
        ),
    ));

    $environment->processing = FALSE;

    foreach ($environment->tasks as &$task) {
      if ($task->task_status == HOSTING_TASK_QUEUED || $task->task_status == HOSTING_TASK_PROCESSING) {
        $environment->active_tasks++;

        if ($task->task_status == HOSTING_TASK_PROCESSING) {
          $environment->processing = TRUE;
        }
      }

      switch ($task->task_status){
        case HOSTING_TASK_SUCCESS:
          $icon = 'check text-success';
          $item_class = 'success';
          break;

        case HOSTING_TASK_ERROR;
          $icon = 'exclamation-circle text-danger';
          $item_class = 'danger';
          break;
        case HOSTING_TASK_WARNING:
          $icon = 'warning text-warning';
          $item_class = 'warning';
          break;

        case HOSTING_TASK_PROCESSING;
        case HOSTING_TASK_QUEUED;
          $icon = 'cog text-info';
          if ($environment->processing) {
            $icon .= ' fa-spin';
          }
          $item_class = 'default';
          break;
      }

      $label = drupal_ucfirst($tasks[$task->task_type]['title']);
      $ago = ' <em class="small">' . format_interval(time() - $task->executed, 1) .' '. t('ago') . '</em>';

      // Override "ago" text.
      if ($task->task_status == HOSTING_TASK_QUEUED) {
        $ago = t('Queued');
      }
      elseif ($task->task_status == HOSTING_TASK_PROCESSING) {
        $ago = t('Running...');
      }

      $text = "<i class='fa fa-{$icon}'></i> {$label} <em class='small'>{$ago}</em>";

      $items[] = l($text, 'node/' . $task->nid, array(
          'html' => TRUE,
          'attributes' => array(
            'class' => "list-group-item list-group-item-{$item_class}",
          ),
      ));

      // Save last task
      if (empty($environment->last_task)) {
        $environment->last_task = array(
          'class' => $item_class,
          'icon' => $icon,
          'ago' => $ago,
          'label' => $label,
          'url' => url("node/$task->nid"),
        );
      }
    }

    // Task Logs
    $environment->task_logs = implode("\n", $items);

    // Determine environment status
    if ($environment->site_status == HOSTING_SITE_DISABLED) {
      $environment->class = 'disabled';
      $environment->list_item_class = 'disabled';
    }
    elseif ($environment->name == $project->settings->live['live_environment']) {
      $environment->class = ' live-environment';
      $environment->list_item_class = 'info';
    }
    else {
      $environment->class = ' normal-environment';
      $environment->list_item_class = 'info';
    }

    if ($environment->active_tasks > 0) {
      $environment->class .= ' active';
    }

    // Pull Request?
    if ($environment->github_pull_request) {
      $environment->class .= ' pull-request';
    }

    // Load Task Links
    $environment->task_links = devshop_environment_links($environment);

  }
  return $node;
}

function devshop_environment_links($environment) {
  global $user;
  $tasks = hosting_available_tasks('site');
  $actions = module_invoke_all('devshop_environment_actions', $environment);

  // Add environment settings link
  $site = node_load($environment->site);
  if (node_access('update', $site)) {
    $items[] = l('<i class="fa fa-sliders"></i> ' . t('Environment Settings'), "node/{$environment->project_nid}/edit/{$environment->name}", array(
      'html' => TRUE,
    ));
    $items[] = '<div class="divider"></div>';
  }

  // Add all actions from hook_devshop_environment_actions()
  foreach ($actions as $action) {
    if (isset($tasks[$action])) {

      // Special handling for delete/disable/enable
      if ($action == 'disable' || $action == 'delete' || $action == 'enable') {
        $tasks[$action]['title'] = '<i class="fa fa-power-off"></i> ' . $tasks[$action]['title'] . ' ' . t('Environment');
      }

      $href = "node/{$environment->site}/site_{$action}";
      $items[] = l($tasks[$action]['title'], $href, array(
          'query' => array(
              'token' => drupal_get_token($user->uid),
          ),
          'html' => TRUE,
      ));
    }
    else {
      $items[] = $action;
    }
  }
  return $items;
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
