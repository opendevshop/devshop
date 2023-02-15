<?php

/**
 * Implements hook_theme()
 */
function boots_theme() {
  return array(
      'environment' => array(
          'arguments' => array(
              'environment' => NULL,
              'project' => NULL,
              'page' => FALSE,
          ),
          'template' => 'environment',
      ),
  );
}

/**
 * Preprocessor for environment template.
 */
function boots_preprocess_environment(&$vars) {
  $environment = &$vars['environment'];
  $project = &$vars['project'];

  // Load last task node.
  if (isset($environment->last_task_nid)) {
    $environment->last_task_node = node_load($environment->last_task_nid);
  }

  if (!isset($environment->git_ref_type)) {
    $environment->git_ref_type = '';
  }

  // Available deploy data targets.
  $vars['target_environments'] = $project->environments;

  // Get token for task links
  global $user;
  $vars['token'] = drupal_get_token($user->uid);

  // Load git refs and create links
  $vars['git_refs'] = array();
  foreach ($project->settings->git['refs'] as $ref => $type) {
    $href = url('hosting_confirm/ENV_NID/site_deploy', array(
        'query' => array(
            'token' => drupal_get_token($user->uid),
            'git_reference' => $ref,
        )
    ));
    $icon = $type == 'tag' ? 'tag' : 'code-fork';

    $vars['git_refs'][$ref] = "<a href='$href'>
        <i class='fa fa-$icon'></i>
        $ref
      </a>";
  }

  // Look for all available source environments
  foreach ($vars['project']->environments as $source_environment) {
    if ($source_environment->site) {
      $vars['source_environments'][$source_environment->name] = $source_environment;
    }
  }

  // Look for remote aliases
  // @TODO: Move to devshop_remotes.module. I could't get devshop_remotes_preprocess_environment() working.
  if (isset($vars['project']->settings->aliases )) {
    foreach ($vars['project']->settings->aliases as $name => $alias) {
      $alias = (object) $alias;
      $alias->site = $name;
      $alias->name = $name;
      $alias->url = $alias->uri;
      $vars['source_environments'][$name] = $alias;
    }
  }

  // Show user Login Link
  if ($environment->site_status == HOSTING_SITE_ENABLED && user_access('create login-reset task')) {
    $environment->login_text = t('Log in');

  }

  // Determine the CSS classes to use.

  // Determine environment status
  if ($environment->site_status == HOSTING_SITE_DELETED) {
    $environment->class = 'deleted';
    $environment->list_item_class = 'deleted';
  }
  elseif ($environment->site_status == HOSTING_SITE_DISABLED) {
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

  // Pull Request?
  if (isset($environment->github_pull_request) && $environment->github_pull_request) {
    $environment->class .= ' pull-request';
    $vars['warnings'][] = array(
      'type' => 'info',
      'icon' => 'github',
      'text' => l($environment->github_pull_request->pull_request_object->title, $environment->github_pull_request->pull_request_object->html_url, array(
        'absolute' => TRUE,
        'attributes' => array(
          'target' => '_blank',
          'title' => t('Visit this Pull Request on GitHub'),
        ),
      )),
    );
  }

  // Load Task Links
  $environment->menu = devshop_environment_menu($environment);
  $environment->menu_rendered = theme("item_list", array(
    'items' => $environment->menu,
    'attributes' => array(
      'class' => array('dropdown-menu dropdown-menu-right'),
      )
    )
  );

  // Task Logs
  $environment->task_count = count($environment->tasks);
  $environment->active_tasks = 0;

  $items = array();

  $environment->processing = FALSE;

  foreach ($environment->tasks_list as $task) {

    if ($task->task_status == HOSTING_TASK_QUEUED || $task->task_status == HOSTING_TASK_PROCESSING) {
      $environment->active_tasks++;

      if ($task->task_status == HOSTING_TASK_PROCESSING) {
        $environment->processing = TRUE;
      }
    }

//    $text = "<i class='fa fa-{$task->icon}'></i> {$task->type_name} <span class='small'>{$task->status_name}</span> <em class='small pull-right'><i class='fa fa-calendar'></i> {$task->ago}</em>";
    $items[] = theme('devshop_task', array('task' => $task));
  }
  $environment->task_logs = l('<i class="fa fa-list"></i> ' . t('Task Logs'), "node/$environment->site/tasks", array(
    'html' => TRUE,
    'attributes' => array(
      'class' => array(
        'list-group-item',
      ),
    ),
  ));
  $environment->task_logs .= '<div class="tasks-wrapper">' . implode("\n", $items) . '</div>';

  // Set a class showing the environment as active.
  if ($environment->active_tasks > 0) {
    $environment->class .= ' active';
  }

  // Check for any potential problems
  // Branch or tag no longer exists in the project.
// @TODO: This fires a false flag too often. Consider removing it.
//  if (!isset($project->settings->git['refs'][$environment->git_ref])) {
//    $vars['warnings'][] = array(
//      'text' =>  t('The git ref %ref is not present in the remote repository.', array(
//        '%ref' => $environment->git_ref,
//      )),
//      'type' => 'warning',
//    );
//  }

  // No hooks configured.
  if (isset($project->settings->deploy) && $project->settings->deploy['allow_environment_deploy_config'] && $environment->site_status == HOSTING_SITE_ENABLED && isset($environment->settings->deploy) && count(array_filter($environment->settings->deploy)) == 0) {
    $vars['warnings'][] = array(
      'text' => t('No deploy hooks are configured. Check !link.', array(
        '!link' => l(t('Environment Settings'), "node/{$environment->site}/edit"),
      )),
      'type' => 'warning',
    );
  }

  // Determine Environment State. Only one of these may be active at a time.
  // State: Platform verify failed.
  if (!empty($environment->tasks['verify']) && current($environment->tasks['verify'])->ref_type == 'platform' && current($environment->tasks['verify'])->task_status == HOSTING_TASK_ERROR) {
    $verify_task = current($environment->tasks['verify']);
    $buttons = l(
      '<i class="fa fa-refresh"></i> ' . t('Retry'),
      "node/{$verify_task->nid}",
      array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn btn-sm text-success'),
        ),
      )
    );
    $buttons .= l(
      '<i class="fa fa-trash"></i> ' . t('Destroy'),
      "hosting_confirm/{$environment->site}/site_delete",
      array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn btn-sm text-danger'),
        ),
        'query' => array(
          'token' => $vars['token'],
        ),
      )
    );
    $vars['warnings'][] = array(
      'text' => t('Codebase verification failed.'),
      'buttons' => $buttons,
      'type' => 'error',
    );
  }

  // State: Site install failed.
  elseif (!empty($environment->tasks['install'])  && current($environment->tasks['install'])->task_status == HOSTING_TASK_ERROR) {
    $install_task = current($environment->tasks['install']);
    $buttons = l(
      '<i class="fa fa-list"></i> ' . t('View Logs'),
      "node/{$install_task->nid}",
      array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn btn-sm text-primary'),
        ),
      )
    );
    $buttons .= l(
      '<i class="fa fa-refresh"></i> ' . t('Retry'),
      "hosting_confirm/{$install_task->rid}/site_{$install_task->task_type}",
      array(
        'html' => TRUE,
        'query' => array(
          'token' => drupal_get_token($user->uid),
        ),
        'attributes' => array(
          'class' => array('btn btn-sm text-success'),
        ),
      )
    );
    $buttons .= l(
      '<i class="fa fa-trash"></i> ' . t('Destroy'),
      "hosting_confirm/{$environment->site}/site_delete",
      array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn btn-sm text-danger'),
        ),
        'query' => array(
          'token' => $vars['token'],
        ),
      )
    );
    $vars['warnings'][] = array(
      'text' => t('Installation failed. The site is not available.'),
      'buttons' => $buttons,
      'type' => 'error',
    );
  }

  // State: Site Install Queued or processing.
  elseif (!empty($environment->tasks['install']) && (current($environment->tasks['install'])->task_status == HOSTING_TASK_QUEUED || current($environment->tasks['install'])->task_status == HOSTING_TASK_PROCESSING)) {

    $vars['warnings'][] = array(
      'text' => t('Environment install in progress!'),
      'type' => 'info',
      'icon' => 'truck',
    );
  }

  // State: Environment Disable Initiated
  elseif (!empty($environment->tasks['disable']) && (current($environment->tasks['disable'])->task_status == HOSTING_TASK_QUEUED || current($environment->tasks['disable'])->task_status == HOSTING_TASK_PROCESSING)) {

    $vars['warnings'][] = array(
      'text' => t('Environment is being disabled.'),
      'type' => 'info',
    );
  }

  // State: Site Delete initiated.
  elseif (!empty($environment->tasks['delete']) || $environment->site_status == HOSTING_SITE_DELETED) {
    $site_delete_task = current($environment->tasks['delete']);
    if (isset($site_delete_task)) {
      $vars['warnings'][] = array(
        'text' => t('Site Destroyed'),
        'type' => 'info',
      );
    }
  }

  // State: Site is Disabled
  elseif ($environment->site_status == HOSTING_SITE_DISABLED) {
    $buttons = '';
    $buttons .= l(
      '<i class="fa fa-power-off"></i> ' . t('Enable'),
      "hosting_confirm/{$environment->site}/site_enable",
      array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn btn-sm text-success'),
        ),
        'query' => array(
          'token' => $vars['token'],
        ),
      )
    );
    $buttons .= l(
      '<i class="fa fa-trash"></i> ' . t('Destroy'),
      "hosting_confirm/{$environment->site}/site_delete",
      array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('btn btn-sm text-danger'),
        ),
        'query' => array(
          'token' => $vars['token'],
        ),
      )
    );

    $vars['warnings']['disabled'] = array(
      'text' => t('Environment is disabled.'),
      'type' => 'info',
      'buttons' => $buttons,
    );
  }

  // State: Site is Deleted
  elseif ($environment->site_status == HOSTING_SITE_DELETED) {
    $buttons = '';
    $vars['warnings']['disabled'] = array(
      'text' => t('Environment was destroyed.'),
      'type' => 'info',
    );
  }

  if (isset($environment->warnings)) {
    foreach ($environment->warnings as $warning) {
      $vars['warnings'][] = array(
        'text' => $warning['text'],
        'type' => $warning['type'],
      );
    }
  }

  // Load user into a variable.
  global $user;
  $vars['user'] = $user;
  $vars['environment'] = $environment;

  // Detect hooks.yml file.
  if (file_exists($environment->repo_path . '/.hooks.yaml')
    || file_exists($environment->repo_path . '/.hooks.yml')
    || file_exists($environment->repo_path . '/.hooks')) {
    $vars['hooks_yml_note'] = t('!file found:');
  }
  else {
    $vars['hooks_yml_note'] = t('Unable to find a file named .hooks, .hooks.yml, or .hooks.yaml. Add one or disable "Run deploy commands in the .hooks file" in project or environment settings.');
  }
  
  //   Load git information
  if (isset($environment->repo_path) && file_exists($environment->repo_path . '/.git')) {
    // Timestamp of last commit.
    $environment->git_last = shell_exec("cd {$environment->repo_path}; git log --pretty=format:'%ct' --max-count=1");
    $environment->git_last_timestamp = date('N');
    $environment->git_last_ago = t('!ago ago', array(
      '!ago' => format_interval(time() - $environment->git_last , 1)
    ));

    // The last commit.
    $environment->git_commit = shell_exec("cd {$environment->repo_path}; git -c color.ui=always log --max-count=1");
    
    // Get the exact SHA
    $environment->git_sha = trim(shell_exec("cd {$environment->repo_path}; git rev-parse HEAD  2> /dev/null"));
    
    // Determine the type of git ref the stored version is
    $stored_git_ref_type = !empty($project->settings->git['refs'][$environment->git_ref_stored])
      ? $project->settings->git['refs'][$environment->git_ref_stored]
      : 'branch';
    $stored_git_sha =  trim(shell_exec("cd {$environment->repo_path}; git rev-parse {$environment->git_ref_stored} 2> /dev/null"));
    
    // Get the actual tag or branch. If a branch and tag have the same SHA, the tag will be output here.
    // "2> /dev/null" ensures errors don't get printed like "fatal: no tag exactly matches".
    $environment->git_ref = trim(str_replace('refs/heads/', '', shell_exec("cd {$environment->repo_path}; git describe --tags --exact-match 2> /dev/null || git symbolic-ref -q HEAD 2> /dev/null")));
    
    $environment->git_ref_type = !empty($project->settings->git['refs'][$environment->git_ref])
      ? $project->settings->git['refs'][$environment->git_ref]
      : 'branch';

    // If the git sha for stored branch are the same, but the type is different, detect if HEAD is detached so we know if this is on a branch or a tag.
    if ($stored_git_sha == $environment->git_sha && $stored_git_ref_type != $environment->git_ref_type) {
      $git_status = shell_exec("cd {$environment->repo_path}; git status");
      if (strpos($git_status, 'On branch ') === 0) {
        $environment->git_ref_type = 'branch';
        $environment->git_ref = $environment->git_ref_stored;
      }
      else {
        $environment->git_ref_type = 'tag';
      }
    }
    
    // Get git status.
    $environment->git_status = trim(shell_exec("cd {$environment->repo_path}; git -c color.ui=always  status"));
    
    // Limit status to 1000 lines
    $lines = explode("\n", $environment->git_status);
    $count = count($lines);
    if ($count > 100) {
      $lines = array_slice($lines, 0, 100);
      $lines[] = "# STATUS TRUNCATED. SHOWING 100 of $count LINES.";
    }
    $environment->git_status  = implode("\n", $lines);
    
    // Get git diff.
    $environment->git_diff = trim(shell_exec("cd {$environment->repo_path}; git -c color.ui=always diff"));
    
    // Limit git diff to 1000 lines
    $lines = explode("\n", $environment->git_diff);
    $count = count($lines);
    if ($count > 1000) {
      $lines = array_slice($lines, 0, 1000);
      $lines[] = "# DIFF TRUNCATED. SHOWING 1000 of $count LINES.";
    }
    $environment->git_diff  = implode("\n", $lines);
  }
  else {
    $environment->git_last = '';
    $environment->git_commit = '';
    $environment->git_sha = '';
    $environment->git_status = '';
    $environment->git_diff = '';
  }

  // Look for alternative git URL
  if ($environment->git_url != $project->git_url) {
    $vars['git_origin'] = $environment->git_url;
  }
}

/**
 * A simple function to output tasks exactly as we need them.
 *
 * Tired of drupal 6 theme system, going as fast as I can.
 *
 * @param $tasks
 *
 * Usage:
 * $tasks = hosting_get_tasks('task_status', HOSTING_TASK_PROCESSING);
 * print boots_render_tasks($tasks);
 */
function boots_render_tasks($tasks = NULL, $class = '', $actions = array(), $float = 'left'){
  global $user;

  if (is_null($tasks)){
    // Tasks
    $tasks = hosting_get_tasks(null, null, 10);
  }

  // Get active or queued
  $tasks_count = 0;
  foreach ($tasks as $task){
    if ($task->task_status == HOSTING_TASK_QUEUED || $task->task_status == HOSTING_TASK_PROCESSING){
      $tasks_count++;
    }
  }

  $task_class = '';
  if ($tasks_count > 0) {
    $task_class = 'active-task fa-spin';
  }

  $items = array();
  $text = '<i class="fa fa-list-alt"></i> '. t('Task Logs');

  // If for an environment, change the link.
  if (!empty($actions)) {

    $environment_node = node_load($tasks[0]->rid);
    $environment = $environment_node->environment;

    $url = "node/{$environment->project_nid}/logs/{$environment->name}";
  }
  else {
    $url = 'hosting/queues/tasks';
  }

  $task_items = array();
  $task_items[] = l($text, $url, array(
    'html' => TRUE,
    'attributes' => array(
      'class' => array(
        'list-group-item',
      ),
    ),
  ));

  $task_types = hosting_available_tasks();

  if (!empty($tasks)) {

    foreach ($tasks as $task) {

      switch ($task->task_status){
        case HOSTING_TASK_SUCCESS:
          $icon = 'check text-success';
          $item_class = 'list-group-item-success';
          break;

        case HOSTING_TASK_ERROR;
          $icon = 'exclamation-circle text-danger';
          $item_class = 'list-group-item-danger';
          break;
        case HOSTING_TASK_WARNING:
          $icon = 'warning text-warning';
          $item_class = 'list-group-item-warning';
          break;

        case HOSTING_TASK_PROCESSING;
        case HOSTING_TASK_QUEUED;
          $icon = 'cog fa-spin text-info';
          $item_class = 'list-group-item-info';
          break;
      }

      $task_node = node_load($task->rid);

      // If environment tasks...
      if (!empty($actions)) {
        $task->title = $task_types[$task_node->type][$task->task_type]['title'];
      }

      $text = '<i class="fa fa-' . $icon . '"></i> ';
      $text .= $task->title;
      $datetime = date('c', $task->changed);
      $text .= ' <time class="timeago task-ago btn-block" datetime="' . $datetime . '">' . format_interval(REQUEST_TIME - $task->changed) .' '. t('ago') . '</time>';

      $id = isset($task_node->environment)? "task-{$task_node->environment->project_name}-{$task_node->environment->name}": "task-";
      $task_items[] = l($text, 'node/' . $task->nid, array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array('list-group-item ' . $item_class),
            'id' => $id,
        ),
      ));
    }
  }
  else {
    $task_items[] = t('No Active Tasks');
  }

  $items[] = array(
    'class' => array('tasks'),
    'data' => '<div class="list-group">' . implode("\n", $task_items) . '</div>',
  );

  if (!empty($actions)) {

    array_unshift($items, array(
      'class' => array('divider'),
    ));

    // Add "Environment Settings" link
    if (node_access('update', $environment_node)) {
      array_unshift($items, l('<i class="fa fa-sliders"></i> ' . t('Environment Settings'), "node/{$environment->site}/edit", array('html' => TRUE)));
    }

    $action_items = array();
    foreach ($actions as $link) {
      $action_items[] = l($link['title'], $link['href'], array(
        'attributes' => array(
          'class' => array('list-group-item'),
        ),
        'query' => array(
          'token' => drupal_get_token($user->uid),
        ),
      ));
    }

    $items[] = array(
      'class' => array('actions'),
      'data' => '<div class="list-group">' . implode("\n", $action_items) . '</div>',
    );
  }

  $tasks = theme('item_list', array(
    'items' => $items,
    'attributes' => array(
      'class' => array(
        'devshop-tasks dropdown-menu dropdown-menu-' . $float,
        )
      ),
      'role' => 'menu',
    )
  );

  if ($tasks_count == 0) {
    $tasks_count = '';
  }

  $logs = t('Task Logs');
  return <<<HTML
    <div class="task-list btn-group">
      <button type="button" class="btn btn-link task-list-button dropdown-toggle $class" data-toggle="dropdown" title="$logs">
        <span class="count">$tasks_count</span>
        <i class="fa fa-gear $task_class"></i>
      </button>
      $tasks
    </div>
HTML;

}

/**
 * Implements hook_preprocess_page()
 * @param $vars
 */
function boots_preprocess_page(&$vars){
//
//  // Add information about the number of sidebars.
//  $has_sidebar_first = !empty($vars['page']['sidebar_first']) || !empty($vars['tabs']);
//  if ($has_sidebar_first && !empty($vars['page']['sidebar_second'])) {
//    if ($vars['node']->type == 'project') {
//      $vars['content_column_class'] = ' class="col-sm-12';
//    }
//    else {
//      $vars['content_column_class'] = ' class="col-sm-9"';
//    }
//  }
//  elseif ($has_sidebar_first || !empty($vars['page']['sidebar_second'])) {
//    $vars['content_column_class'] = ' class="col-sm-9"';
//  }
//  else {
//    $vars['content_column_class'] = ' class="col-sm-12"';
//  }

  if (!empty($vars['tabs']) && (!isset($vars['node']) || $vars['node']->type != 'project')) {
    $vars['content_column_class'] = ' class="col-sm-9"';
  }

  if (user_access('access task logs')){
    $vars['tasks'] = boots_render_tasks();
  }

  // On any node/% page...
  if (isset($vars['node']) || arg(0) == 'node' && is_numeric(arg(1))) {

    if (!isset($vars['node'])) {
      $vars['node'] = node_load(arg(1));
    }

    // Task nodes only have project nid and environment name.
    if (is_numeric($vars['node']->project)) {
      $project_node = node_load($vars['node']->project);
      $vars['node']->project = $project_node->project;

      if (!empty($vars['node']->rid) && isset($project_node->project->environment_nids[$vars['node']->rid])) {
        $vars['node']->environment = $project_node->project->environment_nids[$vars['node']->rid];
      }
    }

    // load $vars['node'] if it's not present (like on node/%/edit)
    if (empty($vars['node'])) {
      $vars['node'] = node_load(arg(1));
    }

    // Set subtitle
    $vars['title'] = $vars['node']->title;
    $vars['title_url'] = "node/" . $vars['node']->nid;

    if (in_array($vars['node']->type, array('site', 'platform', 'project', 'task', 'server', 'client'))){
      $vars['subtitle'] = ucfirst($vars['node']->type);
    }

    // Set title2 if on a node/%/* sub page.
    if (!is_null(arg(2)) && $vars['title'] != $vars['node']->title) {
      $vars['title2'] = $vars['title'];
      $vars['title'] = $vars['node']->title;
    }

    if ($vars['node']->type == 'project'){
      $vars['subtitle'] = t('Project');

      unset($vars['tabs']);

      $vars['title_url'] = "node/" . $vars['node']->nid;
    }

    // Set header and subtitle 2 for nodes that have a project.
    elseif (!empty($vars['node']->project)) {

      $vars['title2'] = $vars['title'];

      if ($vars['node']->type == 'site' || $vars['node']->type == 'platform') {
        $vars['subtitle2'] = t('Environment');
        if ($vars['node']->type == 'platform') {
          $vars['subtitle2'] = t('Environment') . ' ' . t('Platform');
        }
      }
      else {
        $vars['subtitle2'] = ucfirst($vars['node']->type);
      }

      $vars['title'] = $vars['node']->project->name;
      $vars['title_url'] = "node/" . $vars['node']->project->nid;
      $vars['subtitle'] = t('Project');
    }

    // Improve tasks display
    if ($vars['node']->type == 'task') {
      $object = $vars['node']->ref = node_load($vars['node']->rid);

      $vars['title2'] = $object->title;
      if ($object->type == 'site') {
        $vars['subtitle2'] = t('Environment');
      }
      else {
        $vars['subtitle2'] = ucfirst($object->type);
      }

      // Only show environment name if site is in project.
      if (isset($object->environment)) {
        $vars['title2'] = $object->environment->name;
      }

      if (!empty($object->environment->site)) {
        $vars['title2_url'] = 'node/' . $object->environment->site;
      }
      else {
        $vars['title2_url'] = 'node/' . $object->nid;
      }
      $vars['title2'] = l($vars['title2'], $vars['title2_url']);

      if ($vars['subtitle2'] == 'Platform') {
        $vars['subtitle2'] = t('Environment') . ' ' . t('Platform');
      }
    }

    // For node/%/* pages where node is site, use the environment name as title2
    if (isset($vars['node']->environment)){
      // If the project node creation process was interrupted, the environment will have no "site" nid.
      if ($vars['node']->environment->site == 0) {
        $vars['title2_url'] = url('node/' . $vars['node']->environment->project_nid);
      }
      else {
        $vars['title2_url'] = url('node/' . $vars['node']->environment->site);
      }

      // Override subtitle2 for platform node and task pages.
      if ($vars['node']->type == 'platform') {
        $vars['subtitle2'] = ' / '. l(t('Platform'), 'node/' . $vars['node']->nid);
      }
      elseif ($vars['node']->type == 'task' && $vars['node']->ref->type == 'platform') {
        $vars['subtitle2'] = ' / '. l(t('Platform'), 'node/' . $vars['node']->rid);
      }

      $vars['title2'] = l($vars['node']->environment->name, $vars['title2_url']);
    }

    // For node/%/* pages where node is site or platform, use the environment name as title2
    if (($vars['node']->type == 'site' || $vars['node']->type == 'platform')&& isset($vars['node']->environment)){

      $vars['title2_url'] = url('node/' . $vars['node']->environment->site);
      $vars['title2'] = l($vars['node']->environment->name, $vars['title2_url']);
    }
    // On environment settings page...
    if (arg(0) == 'node' && arg(3) == 'env') {
      $project_node = node_load(arg(1));
      $environment = $project_node->project->environments[arg(4)];

      $vars['title2_url'] = 'node/' . $environment->site;
      $vars['title2'] = l($environment->name, $vars['title2_url']);
      $vars['subtitle2'] = t('Environment');
    }

    $vars['title'] = l($vars['title'], $vars['title_url']);

  }

  if (arg(0) == 'hosting_confirm') {
    $node = node_load(arg(1));

    if (isset($node->project)) {
      if ($node->type == 'project') {
        $project_node = $node;
      }
      else {
        $project_node = node_load($node->project->nid);
        $title2text = $node->environment->name;
        $title2url = 'node/' . $node->environment->site;
      }
      $vars['title'] = $project_node->title;
      $vars['title_url'] = "node/{$project_node->nid}";
      $vars['subtitle'] = t('Project');
      $vars['title'] = l($vars['title'], $vars['title_url']);

      $vars['title2'] = l($title2text, $title2url);
      $vars['subtitle2'] = t('Environment');

      $vars['title_prefix']['title3'] = array(
        '#markup' => drupal_get_title(),
        '#prefix' => '<h4>',
        '#suffix' => '</h4>',
      );
    }
  }

  // Set title on create environment page.
  if (arg(0) == 'node' && arg(2) == 'site' && !empty(arg(3))) {

    // Look for project. If there is none, return.
    $project_node = devshop_projects_load_by_name(arg(3));
    if ($project_node->type == 'project') {
      $vars['title'] = l($project_node->title, "node/$project_node->nid");
      $vars['subtitle'] = t('Project');
    }
  }

  if (variable_get('devshop_support_widget_enable', TRUE)) {
    drupal_add_js(drupal_get_path('theme', 'boots') . '/js/intercomSettings.js', array('type' => 'file'));
  }

  // Render stuff
  $vars['tabs_rendered'] = render($vars['tabs']);
  $vars['sidebar_first_rendered'] = render($vars['page']['sidebar_first']);

  // Indicate the project is inactive.
  if (isset($project_node) && isset($project_node->nid) && (int) $project_node->status == NODE_NOT_PUBLISHED) {
    $vars['subtitle'] .= ' <small>' . t('Inactive Project') .  '</small>';
  }
}


/**
 *
 * @param $vars
 */
function boots_preprocess_node(&$vars) {
  global $user;
  if ($vars['node']->type == 'project') {
    boots_preprocess_node_project($vars);
  }
  elseif ($vars['node']->type == 'task') {
    boots_preprocess_node_task($vars);
  }
  elseif ($vars['node']->type == 'site') {
    if (!empty($vars['node']->environment)) {
      $vars['theme_hook_suggestions'][] =  'node__site_environment';
    }
  }

  // I don't know why, but server nodes are missing their titles!
  if (isset($vars['node']->nid) && empty($vars['node']->title)) {
    $vars['node'] = node_load($vars['node']->nid);
  }
}

/**
 * Preprocessor for Project Nodes.
 * @param $vars
 */
function boots_preprocess_node_task(&$vars) {
  global $user;
  $node = $vars['node'];

  if ($node->task_status == HOSTING_TASK_QUEUED || $node->task_status == HOSTING_TASK_PROCESSING) {
    $vars['cancel_button'] = l(t('Cancel'), "hosting/tasks/{$node->nid}/cancel", array(
      'attributes' => array('class' => array('btn btn-default')),
      'query' => array(
          'token' => drupal_get_token($user->uid),
        ),
    ));
  }

  $vars['retry']['#attributes']['class'][] = 'retry btn btn-default';
  $vars['retry']['#prefix'] = '';
  $vars['retry']['#suffix'] = '';
}

/**
 * Preprocessor for Project Nodes.
 * @param $vars
 */
function boots_preprocess_node_project(&$vars){
  global $user;

  // Easy Access
  $node = &$vars['node'];
  $project = $vars['project'] = $vars['node']->project;

  // Live Domain link.
  if (isset($project->settings->live['live_domain']) && $project->settings->live['live_domain']) {
    $vars['live_domain_url'] =  'http://' . $project->settings->live['live_domain'];
    $vars['live_domain_text'] =  'http://' . $project->settings->live['live_domain'];
  }
  else {
    $vars['live_domain_url'] =  '';
  }

  $vars['git_refs'] = array();

  if (empty($node->project->settings->git['refs'])){
    $vars['deploy_label'] = '';

    if ($node->verify->task_status == HOSTING_TASK_ERROR) {
      $vars['deploy_label'] = t('There was a problem refreshing branches and tags.');
      $vars['git_refs'][] = l(t('View task log'), 'node/' . $node->verify->nid);
      $link_refresh = l(t('Refresh branches'), 'hosting_confirm/' . $node->nid . '/project_verify', array('attributes' => array('class' => array('refresh-link')), 'query' => array('token' => drupal_get_token($user->uid))));
      array_unshift($vars['git_refs'], $link_refresh);
    }
    elseif ($node->verify->task_status == HOSTING_TASK_QUEUED || $node->verify->task_status == HOSTING_TASK_PROCESSING) {
      $vars['deploy_label'] =  t('Branches refreshing.  Please wait.');
    }
  }
  else {
    $vars['deploy_label'] = t('Deploy a tag or branch');

    foreach ($node->project->settings->git['refs'] as $ref => $type){
      $href = url('hosting_confirm/ENV_NID/site_deploy', array(
        'query' =>array(
          'token' => drupal_get_token($user->uid),
          'git_reference' => $ref,
        )
      ));
      $icon = $type == 'tag'? 'tag': 'code-fork';

      $vars['git_refs'][$ref] = "<a href='$href'>
        <i class='fa fa-$icon'></i>
        $ref
      </a>";
    }
  }

  // Get available servers
  $vars['web_servers'] = hosting_get_servers('http', FALSE);
  $vars['db_servers'] = hosting_get_servers('db', FALSE);

  // React to git provider
  if ($project->git_provider == 'github') {
    $url = strtr($project->git_url, array(
      'git@github.com:' => 'http://github.com/',
      '.git' => '',
    ));
    if (empty($project->settings->deploy['last_webhook'])){
      $url .= '/settings/hooks/new';
    }
    else {
      $url .= '/settings/hooks';
    }
    $vars['add_webhook_url'] = $url;
    $vars['add_webhook_icon'] = 'github';
  }
  else {
    $vars['add_webhook_url'] = '#';
    $vars['add_webhook_icon'] = 'warning';
  }

  // Set webhook interval
  if ($project->settings->deploy['method'] == 'webhook' && $project->settings->deploy['last_webhook']){
    $interval = format_interval(REQUEST_TIME - $project->settings->deploy['last_webhook']);
    $vars['webhook_ago'] = t('@time ago', array('@time' => $interval));
  }

  if ($project->settings->deploy['method'] == 'queue') {
    $vars['queued_ago'] = hosting_format_interval(variable_get('hosting_queue_pull_last_run', FALSE));
  }

  // Webhook status output.
  if (empty($node->project->settings->deploy['last_webhook'])) {
    $button_text = t('Setup Webhook');
    $class = 'btn-warning';
  }
  else {
    $button_text =  t('Webhook URL');
    $class = 'text-muted';
  }

  $title = t('Webhook for project %name', array('%name' => $node->title));
  $prefix = t('Deploy code to your environments with an incoming webhook with the following URL:');

  if ($project->git_provider == 'github') {
    $suffix = t('GitHub will ping this URL after each code push to keep the servers up to date, and can create environments on Pull Request.');
    $suffix2 = t('Copy the link above, then click the link below to go to the webhooks page for this project.');
    $suffix3 = t('DevShop only has support for Push and Pull Request events.  Set content type to <em>application/json</em>.');

    if (empty($project->settings->deploy['last_webhook'])){
      $github_button_text = t('Add a Webhook at GitHub.com');
    }
    else {
      $github_button_text = t('Manage Webhooks at GitHub.com');
    }
    $button = l($github_button_text, $vars['add_webhook_url'], array('attributes'=> array('class' => array('btn btn-primary'), 'target' => '_blank')));
  }
  else {
    $suffix = t('Ping this URL after each code push to keep the servers up to date.');
    $button = '';
    //@TODO: Link to more help such as example scripts.
  }

  $url =  $node->project->webhook_url;
  $project_name = $node->title;

  // Only show the webhook url to those who can create projects.
  if (user_access('create project')) {
    $vars['webhook_url'] = <<<HTML

            <a href="#" data-toggle="modal" class="btn btn-xs $class"
data-target="#webhook-modal" title="Webhook URL">
              <i class="fa fa-chain"></i> $button_text
            </a>

            <!-- Modal -->
            <div class="modal fade" id="webhook-modal" tabindex="-1" role="dialog" aria-labelledby="webhook-modal" aria-hidden="true">
              <div class="modal-dialog">
                <div class="modal-content">
                  <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">&times;</span><span class="sr-only">Close</span></button>
                    <h4 class="modal-title" id="drush-alias-modal">$title</h4>
                  </div>
                  <div class="modal-body">
                  <p>
                    $prefix
                  </p>
                  <p><input id="webhook-url" class="form-control" value="$url" onclick="this.select()"></p>
                  <p>
                    $suffix
                  </p>
                  <p>
                    $suffix2
                  </p>
                  <p>
                    $suffix3
                  </p>
                  $button
                  </div>
                  <div class="modal-footer">
                    <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
                  </div>
                </div>
              </div>
            </div>
HTML;
  }
  else {
    $vars['webhook_url'] = '';
  }
  $vars['hosting_queue_admin_link'] = l(t('Configure Queues'), 'admin/hosting/queues');

  // Available deploy data targets.
  $vars['target_environments'] = $project->environments;

  // Prepare environments output
  // Render live environment first.
  if (isset($project->settings->live) && $project->settings->live['live_environment'] && isset($project->environments[$project->settings->live['live_environment']])) {
    $environments = [$project->environments[$project->settings->live['live_environment']]];
    
    $remaining = $vars['node']->project->environments;
    unset($remaining[$project->settings->live['live_environment']]);
    $environments += $remaining;
  }
  foreach ($environments as $environment) {

    // Render each environment.
    $vars['environments'][] = theme('environment', array(
      'environment' => $environment,
      'project' => $vars['node']->project,
    ));
  }

  // Warnings & Errors
  // If environment-specific deploy hooks is not allowed and there are no default deploy hooks, warn the user
  // that they will have to manually run updates.
  if (isset($project->messages)) {
    $vars['project_messages'] = $project->messages;
  }

  if (isset($vars['node']->project->settings->deploy['default_hooks']) && !$vars['node']->project->settings->deploy['allow_environment_deploy_config'] && count(array_filter($vars['node']->project->settings->deploy['default_hooks'])) == 0) {
    $vars['project_messages'][] = array(
      'message' => t('No deploy hooks are configured for this project. If new code is deployed, you will have to run update.php manually. Check your !link.', array(
        '!link' => l(t('Project Settings'),"node/{$vars['node']->nid}/edit"),
      )),
      'icon' => '<i class="fa fa-exclamation-triangle"></i>',
      'type' => 'warning',
    );
  }

  if (!isset($vars['project_extra_items']) || !is_array($vars['project_extra_items'])) {
    $vars['project_extra_items'] = array();
  }
}

/**
 * Returns HTML for primary and secondary local tasks.
 *
 * @param array $variables
 *   An associative array containing:
 *     - primary: (optional) An array of local tasks (tabs).
 *     - secondary: (optional) An array of local tasks (tabs).
 *
 * @return string
 *   The constructed HTML.
 *
 * @see theme_menu_local_tasks()
 * @see menu_local_tasks()
 *
 * @ingroup theme_functions
 */
function boots_menu_local_tasks(&$variables) {
  $output = '';

  if (!empty($variables['primary'])) {
    $variables['primary']['#prefix'] = '<h2 class="element-invisible">' . t('Primary tabs') . '</h2>';
    $variables['primary']['#prefix'] .= '<ul class="tabs--primary nav nav-pills nav-stacked">';
    $variables['primary']['#suffix'] = '</ul>';
    $output .= drupal_render($variables['primary']);
  }

  if (!empty($variables['secondary'])) {
    $variables['secondary']['#prefix'] = '<h2 class="element-invisible">' . t('Secondary tabs') . '</h2>';
    $variables['secondary']['#prefix'] .= '<ul class="tabs--secondary pagination pagination-sm">';
    $variables['secondary']['#suffix'] = '</ul>';
    $output .= drupal_render($variables['secondary']);
  }

  return $output;
}

/**
 * Implements hook_form_FORM_ID_alter() for node_site_form
 *
 * "Environment" Settings form.
 */
function boots_form_project_node_form_alter(&$form, &$form_state, $form_id) {

  $form['#prefix'] = '<h3>' . t('Project Settings') . '</h3>';

}
