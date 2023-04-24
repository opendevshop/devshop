<?php

/**
 * Projects Page
 *
 * Callback that is invoked when the user points the brower to
 * projects. It displays a nice tabulated list of projects
 * and cool things you can do with them.
 *
 */
function devshop_projects_projects_page($show_inactive = FALSE) {
  drupal_set_title(t('Projects'));
  $tasks = hosting_available_tasks('site');
  $query = db_select('hosting_devshop_project', 'p')
    ->fields('p', array('nid'))
  ;
  $query->leftJoin('node', 'n', "p.nid = n.nid");

  $results = $query->execute();

  $rows = array();
  $inactive_count = 0;

  foreach ($results as $result) {
    $project_node = node_load($result->nid);
    $project = $project_node->project;

    // Make sure user can view the project node.
    if (!node_access('view', $project_node)) {
      continue;
    }

    // If project is unpublished, and the "show inactive" filter is not on, hide it from the list.
    if ($project_node->status == NODE_NOT_PUBLISHED && !$show_inactive) {
      $inactive_count++;
      continue;
    }

    // Load an environment for extra info like version.
    $first_environment = current($project->environments);
    if ($first_environment) {
      $platform_node = node_load($first_environment->platform);
      $version = $platform_node->release->version ?? t('unknown');
    }
    else {
      $version = '';
    }
      $row = array();

      // Link to Project page
      $url = url("node/$project->nid");
      $title = t('View dashboard for @project project', array(
        '@project' => $project_node->title
      ));
      $first_col = <<<HTML

<a href="$url" title="$title" class="project-link project_$project->nid">
    $project->name
</a>

HTML;

      // Live URL
      $live_domain = $project->settings->live['live_domain'];
      $live_environment = $project->settings->primary_environment;

      if ($live_domain) {
        $domain = $live_domain;
      }
      elseif (isset($project->environments[$live_environment])) {
        $domain = $project->environments[$live_environment]->uri;
      }
      elseif (count($project->environments)) {
        $domain = current($project->environments)->uri;
      }
      else {
        $domain = '';
        $first_col .= '<span class="small live-link text-muted">' . t('No Environments') . '</span>';
      }

      $first_col .= l($domain, "http://$domain", array(
        'attributes' => array(
          'class' => array('small live-link text-muted'),
          'target' => '_blank',
        ),
      ));

      $row[] = $first_col;

      // Drupal Version
      $row[] = "<span class='text-muted'><i class='fa fa-drupal'></i>
 $version 
      <br />
      <small class='install-profile'>{$project->hosting_settings['install']['profile']}</small>
</button>";

    // Git URL
    $row[] = strtr("<input type='textfield' value='!url' onclick='this.select();'/>", array('!url' => scrub_repo_url($project->git_url)));

    // Links to all environments
    $items = '';
    foreach ($project->environments as $env => $environment) {
      if (empty($environment->last_task_node)) {
        $icon = 'cube';
        $class = 'status';
      }
      else {
        if ($environment->last_task_node->task_status == HOSTING_TASK_SUCCESS) {
          $icon = 'check';
          $class = 'success';
        }
        elseif ($environment->last_task_node->task_status == HOSTING_TASK_QUEUED) {
          $icon = 'gear';
          $class = 'status';
        }
        elseif ($environment->last_task_node->task_status == HOSTING_TASK_PROCESSING) {
          $icon = 'gear fa-spin';
          $class = 'status active';
        }
        elseif ($environment->last_task_node->task_status == HOSTING_TASK_WARNING) {
          $icon = 'exclamation-triangle';
          $class = 'warning';
        }
        elseif ($environment->last_task_node->task_status == HOSTING_TASK_ERROR) {
          $icon = 'exclamation-circle';
          $class = 'danger';
        }
      }

      $nid = $environment->site ? $environment->site : $environment->platform;
      $items .= l("<i class='fa fa-{$icon}'></i> " . $environment->name, "node/{$nid}", array(
        'attributes' => array(
            'class' => array('btn btn-sm alert-' . $class),
            'title' => $tasks[$environment->last_task_node->task_type]['title'] . ': ' . _hosting_parse_error_code($environment->last_task_node->task_status),
            'id' => 'badge-' . $environment->project_name . '-' . $environment->name,
        ),
        'html' => TRUE,
      ));
    }
    $count = count($project->environments);
    $row[] = <<<HTML
<div class="project-environments btn-toolbar" role="toolbar">
  <div class="btn-group btn-group-xs" role="group">
    $items
  </div>
</div>

HTML;




    $rows[] = $row;
  }

  // No Projects
  if (empty($rows)) {
    $link = url('admin/hosting');
    $welcome = t('Welcome to <em>your</em> DevShop!');
    $subtitle = t("You are almost ready to start working.");
    $text = t("Check your DevShop settings or get started with a new Project.");
    $button = t('DevShop Settings');

    $link_create = url('projects/add');
    $button_create = t('Create your first Project');

    $output = <<<HTML
<div class="jumbotron">
  <h1>$welcome</h1>
  <h3>$subtitle</h3>
  <p class="text-muted">$text</p>
  <p>
   <a class="btn btn-primary btn-lg" href="$link" role="button"><i class="fa fa-gear"></i> $button </a>
   <a class="btn btn-success btn-lg" href="$link_create" role="button"><i class="fa fa-plus"></i> $button_create </a>
  </p>
</div>

HTML;
  }
  else {
    $output = theme('table', array('rows' => $rows, 'attributes' => array('class' => array('table'))));
  }
  return $output;
}

/**
 * Count the number of projects.
 * @return mixed
 */
function devshop_projects_count($status = 1) {
  $query = db_select('node', 'n')
    ->fields('n', array('nid'))
    ->condition('n.type', 'project')
    ->condition('n.status', $status)
    ->countQuery()
  ;
  return $query->execute()->fetchField();
}

/**
 *
 */
function devshop_projects_inactive_tab_title() {
  static $count;
  if (!isset($count)) {
    $count = devshop_projects_count(NODE_NOT_PUBLISHED);
  }

  return "Inactive Projects: $count";
}

/**
 * Access check for "inactive projects" tab.
 */
function devshop_projects_inactive_tab_access() {
  if (arg(0) == 'projects' && arg(1) == 'add') {
    return FALSE;
  }
  static $count;
  if (!isset($count)) {
    $count = devshop_projects_count(NODE_NOT_PUBLISHED);
  }
  return $count;
}



/**
 * Implementation of hook_view().
 *
 * Project Page Display
 */
function devshop_projects_node_view($node, $view_mode, $langcode) {
  global $user;

  if ($node->type != 'project') {
    return;
  }

  $project = &$node->project;

  // Check to see if this project is still in the wizard
  ctools_include('object-cache');
  $project_wizard_cache = ctools_object_cache_get('project', 'devshop_project');
  if (current_path() == "node/{$node->nid}" && isset($project_wizard_cache->nid) && $node->nid == $project_wizard_cache->nid) {
    drupal_goto('projects/add/' . $project_wizard_cache->step);
  }
}

/**
 * Generates the Environment Menu item list.
 *
 * @param $environment
 * @return array
 */
function devshop_environment_menu($environment) {
  global $user;
  $tasks = hosting_available_tasks('site');
  $first_items = devshop_environment_menu_first_items($environment, $tasks);
  $contrib_items = module_invoke_all('devshop_environment_menu', $environment, $tasks);

  $items = array_merge($first_items, $contrib_items);

  // Last menu items
  if ($environment->site && $environment->site_status == HOSTING_SITE_ENABLED) {

    if (!empty($contrib_items)) {
      $items[] = '<div class="divider"></div>';
    }

    $items[] = 'verify';
    $items[] = 'flush_cache';
    $items[] = 'run_cron';
    $items[] = 'update';
    $items[] = '<div class="divider"></div>';

    // Backup & Restore
    if (drupal_valid_path("node/$environment->site/backups")) {
      $items[] = l(t('Backups'), "node/$environment->site/backups");
    }
  }

  drupal_alter('devshop_environment_menu', $items, $environment);

  // Add all actions from hook_devshop_environment_menu()
  foreach ($items as $item) {
      if (isset($tasks[$item])) {

      // Special handling for delete/disable/enable
      if ($item == 'disable' || $item == 'enable') {
        $tasks[$item]['title'] = '<i class="fa fa-power-off"></i> ' . $tasks[$item]['title'] . ' ' . t('Environment');
      }
      elseif ($item == 'delete') {
        $tasks[$item]['title'] = '<i class="fa fa-trash-o"></i> ' . t('Destroy Environment');
      }

      $href = "hosting_confirm/{$environment->site}/site_{$item}";
      $options = array(
          'query' => array(
              'token' => drupal_get_token($user->uid),
          )
      );

      // If on the site node page, set destination to be the site node.
      if (arg(0) == 'node' && arg(1) == $environment->site) {
        $options['query']['destination'] = current_path();
      }

      $url = url($href, $options);
      $title = $tasks[$item]['title'];
      if (isset($tasks[$item]['icon'])) {
        $icon = '<i class="fa fa-' . $tasks[$item]['icon'] . '""></i>';
      }
      else {
        $icon = '';
      }

      // Trick hosting_task_menu_access_csrf() into seeing the token.
      $_GET['token'] = drupal_get_token($user->uid);
      if (drupal_valid_path($href)) {
          $output_items[] = <<<HTML
      <a href="$url">
            $icon $title
      </a>
HTML;
      }
    }
    else {
      $output_items[] = $item;
    }
  }
  return $output_items;
}

/**
 * Implements hook_nodeapi_TYPE_OP() for site nodes and view op.
 */
function devshop_projects_nodeapi_site_view(&$node, $a3, $a4) {
  if (!empty($node->project)) {
    // Display Project, Environment and Branch.
    $node->content['info']['project'] = array(
      '#type' => 'item',
      '#title' => t('Project'),
      '#markup' => l($node->project->name, "node/{$node->project->nid}"),
      '#weight' => -12,
    );
    $node->content['info']['env'] = array(
      '#type' => 'item',
      '#title' => t('Environment'),
      '#markup' => $node->environment->name,
      '#weight' => -11,
    );
    $node->content['info']['branch'] = array(
      '#type' => 'item',
      '#title' => t('Branch/Tag'),
      '#markup' => $node->environment->git_ref,
      '#weight' => -11,
    );

    // Add Breadcrumbs
    $crumbs = array();
    $crumbs[] = l(t('Home'), '<front>');
    $crumbs[] = l(t('Projects'), 'projects');
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
  if (!isset($node->task_args)) {
    $node->task_args = array();
  }
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
    '#markup' => l($task_host->project->name, "node/{$task_host->project->nid}"),
    '#weight' => -12,
    '#access' => isset($task_host->project),
  );
  $node->content['info']['parameters'] = array(
    '#type' => 'item',
    '#title' => t('Task Parameters'),
    '#markup' => theme('table', array('header' => array(), 'rows' =>   $rows)),
    '#weight' => -11,
    '#access' => count($rows),
  );

}

/**
 * Our own version of _hosting_site_goto_link()
 */
function devshop_hosting_site_goto_link($node) {
//  $project = node_load($node->environment->project_nid);

  // If this is the live environment, use the live domain.
  if ($node->environment->name == $node->project->settings->primary_environment) {
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
  if (!is_null($cache) && (REQUEST_TIME < $cache->data['expire'])) {
    $title = t("Log in: !url", array('!url' => $url));
  }
  else {
    $title = t("!url", array('!url' => $url));
  }
  $options['attributes']['target'] = '_blank';
  $options['attributes']['class'] = 'hosting-goto-site-link';
  return l($title, "node/" . $node->nid . "/goto_site", $options);
}
