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
function boots_preprocess_environment(&$vars)
{
  $environment = &$vars['environment'];
  $project = &$vars['project'];

  // Load git refs and create links
  $vars['git_refs'] = array();
  foreach ($project->settings->git['refs'] as $ref => $type) {
    $href = url('node/ENV_NID/site_devshop-deploy', array(
        'query' => array(
            'git_ref' => $ref,
        )
    ));
    $icon = $type == 'tag' ? 'tag' : 'code-fork';

    $vars['git_refs'][$ref] = "<a href='$href'>
        <i class='fa fa-$icon'></i>
        $ref
      </a>";
  }

  // Look for all available source environments
  foreach ($vars['project']->environments as &$source_environment) {
    if ($source_environment->site) {
      $vars['source_environments'][$source_environment->name] = $source_environment;
    }
  }

  // Show user Login Link
  if ($environment->site_status == HOSTING_SITE_ENABLED && user_access('create login-reset task')) {
    $environment->login_text = t('Log in');

  }

  // Determine the CSS classes to use.

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

  // Pull Request?
  if ($environment->github_pull_request) {
    $environment->class .= ' pull-request';
  }

  // Load Task Links
  $environment->task_links = devshop_environment_links($environment);
  $environment->task_links_rendered = theme("item_list", array(
    'items' => $environment->task_links,
    'attributes' => array(
      'class' => array('dropdown-menu dropdown-menu-right'),
      )
    )
  );

  // Task Logs
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

  foreach ($environment->tasks_list as $task) {

    if ($task->task_status == HOSTING_TASK_QUEUED || $task->task_status == HOSTING_TASK_PROCESSING) {
      $environment->active_tasks++;

      if ($task->task_status == HOSTING_TASK_PROCESSING) {
        $environment->processing = TRUE;
      }
    }

    $text = "<i class='fa fa-{$task->icon}'></i> {$task->type_name} <span class='small'>{$task->status_name}</span> <em class='small pull-right'><i class='fa fa-calendar'></i> {$task->ago}</em>";

    $items[] = l($text, "node/{$task->nid}/revisions/{$task->vid}/view", array(
        'html' => TRUE,
        'attributes' => array(
            'class' => "list-group-item list-group-item-{$task->status_class}",
        ),
    ));
    $environment->task_logs = implode("\n", $items);
  }

  // Set a class showing the environment as active.
  if ($environment->active_tasks > 0) {
    $environment->class .= ' active';
  }

  // Check for any potential problems
  // Branch or tag no longer exists in the project.
  if (!isset($project->settings->git['refs'][$environment->git_ref])) {
    $vars['warnings'][] = array(
      'text' =>  t('The git reference %ref is no longer available.', array(
        '%ref' => $environment->git_ref,
        '@type' => $environment->git_ref_type,
      )),
      'type' => 'error',
    );
  }

  // No hooks configured.
  if ($project->settings->deploy['allow_environment_deploy_config'] && $environment->site_status == HOSTING_SITE_ENABLED && count(array_filter($environment->settings->deploy)) == 0) {
    $vars['warnings'][] = array(
      'text' => t('No deploy hooks are configured. Check your !link.', array(
        '!link' => l(t('Environment Settings'), "node/{$project->nid}/edit/{$environment->nid}"),
      )),
      'type' => 'warning',
    );
  }

  // Load user into a variable.
  global $user;
  $vars['user'] = $user;

  // Get token for task links
  $vars['token'] = drupal_get_token($user->uid);
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
      'class' => 'list-group-item',
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
      $text .= ' <small class="task-ago btn-block">' . format_interval(time() - $task->changed) .' '. t('ago') . '</small>';

      $task_items[] = l($text, 'node/' . $task->nid, array(
        'html' => TRUE,
        'attributes' => array(
          'class' => 'list-group-item ' . $item_class,
            'id' => "task-{$task_node->environment->project_name}-{$task_node->environment->name}",
        ),
      ));
    }
  }
  else {
    $task_items[] = t('No Active Tasks');
  }

  $items[] = array(
    'class' => 'tasks',
    'data' => '<div class="list-group">' . implode("\n", $task_items) . '</div>',
  );

  if (!empty($actions)) {

    array_unshift($items, array(
      'class' => 'divider',
    ));

    // Add "Environment Settings" link
    if (node_access('update', $environment_node)) {
      array_unshift($items, l('<i class="fa fa-sliders"></i> ' . t('Environment Settings'), "node/{$environment->site}/edit", array('html' => TRUE)));
    }

    $action_items = array();
    foreach ($actions as $link) {
      $action_items[] = l($link['title'], $link['href'], array(
        'attributes' => array(
          'class' => 'list-group-item',
        ),
        'query' => array(
          'token' => drupal_get_token($user->uid),
        ),
      ));
    }

    $items[] = array(
      'class' => 'actions',
      'data' => '<div class="list-group">' . implode("\n", $action_items) . '</div>',
    );
  }

  $tasks = theme('item_list', $items, '', 'ul', array('class' => 'devshop-tasks dropdown-menu dropdown-menu-' . $float, 'role' => 'menu'));

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

  // Add information about the number of sidebars.
  $has_sidebar_first = !empty($vars['page']['sidebar_first']) || !empty($vars['tabs']);
  if ($has_sidebar_first && !empty($vars['page']['sidebar_second'])) {
    $vars['content_column_class'] = ' class="col-sm-6"';
  }
  elseif ($has_sidebar_first || !empty($vars['page']['sidebar_second'])) {
    $vars['content_column_class'] = ' class="col-sm-9"';
  }
  else {
    $vars['content_column_class'] = ' class="col-sm-12"';
  }

  if (user_access('access task logs')){
    $vars['tasks'] = boots_render_tasks();
  }

  // On any node/% page...
  if ($vars['node'] || arg(0) == 'node' && is_numeric(arg(1))) {

    // load $vars['node'] if it's not present (like on node/%/edit)
    if (empty($vars['node'])) {
      $vars['node'] = node_load(arg(1));
    }

    // Set subtitle
    $vars['title'] = $vars['node']->title;
    $vars['subtitle'] = ucfirst($vars['node']->type);
    $vars['title_url'] = "node/" . $vars['node']->nid;

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
    elseif (isset($vars['node']->project)) {

      $vars['title2'] = $vars['title'];

      if ($vars['node']->type == 'site') {
        $vars['subtitle2'] = t('Environment');
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
      $object = node_load($vars['node']->rid);

      $vars['title2'] = $object->title;
      if ($object->type == 'site') {
        $vars['subtitle2'] = t('Environment');
      }
      else {
        $vars['subtitle2'] = ucfirst($object->type);
      }

      // Only show environment name if site is in project.
      if (isset($object->project)) {
        $vars['title2'] = $object->environment->name;
      }

      $vars['title2_url'] = 'node/' . $object->nid;
      $vars['title2'] = l($vars['title2'], $vars['title2_url']);

    }


    // For node/%/* pages where node is site, use the environment name as title2
    if ($vars['node']->type == 'site' && isset($vars['node']->environment)){

      $vars['title2_url'] = 'node/' . $vars['node']->nid;
      $vars['title2'] = l($vars['node']->environment->name, $vars['title2_url']);

    }

    $vars['title'] = l($vars['title'], $vars['title_url']);

  }
  if (variable_get('devshop_support_widget_enable', TRUE)) {
    $vars['closure'] .= <<<HTML
<script>
  window.intercomSettings = {
    app_id: "ufeta82d"
  };
</script>
<script>(function(){var w=window;var ic=w.Intercom;if(typeof ic==="function"){ic('reattach_activator');ic('update',intercomSettings);}else{var d=document;var i=function(){i.c(arguments)};i.q=[];i.c=function(args){i.q.push(args)};w.Intercom=i;function l(){var s=d.createElement('script');s.type='text/javascript';s.async=true;s.src='https://widget.intercom.io/widget/ufeta82d';var x=d.getElementsByTagName('script')[0];x.parentNode.insertBefore(s,x);}if(w.attachEvent){w.attachEvent('onload',l);}else{w.addEventListener('load',l,false);}}})()</script>
HTML;
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
}

/**
 * Preprocessor for Project Nodes.
 * @param $vars
 */
function boots_preprocess_node_task(&$vars) {

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
  if ($project->settings->live['live_domain']) {
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
      $vars['git_refs'][] = l(t('Refresh branches'), 'node/' . $node->nid . '/project_verify', array('attributes' => array('class' => 'refresh-link'), 'query' => array('token' => drupal_get_token($user->uid))));
    }
    elseif ($node->verify->task_status == HOSTING_TASK_QUEUED || $node->verify->task_status == HOSTING_TASK_PROCESSING) {
      $vars['deploy_label'] =  t('Branches refreshing.  Please wait.');
    }
  }
  else {
    $vars['deploy_label'] = t('Deploy a tag or branch');

    foreach ($node->project->settings->git['refs'] as $ref => $type){
      $href = url('node/ENV_NID/site_devshop-deploy', array(
        'query' =>array(
          'git_ref' => $ref,
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
  $vars['web_servers'] = hosting_get_servers('http');
  $vars['db_servers'] = hosting_get_servers('db');

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
    $interval = format_interval(time() - $project->settings->deploy['last_webhook']);
    $vars['webhook_ago'] = t('@time ago', array('@time' => $interval));
  }

  if ($project->settings->deploy['method'] == 'queue') {
    $vars['queued_ago'] = hosting_format_interval(variable_get('hosting_queue_deploy_last_run', FALSE));
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
    $button = l($github_button_text, $vars['add_webhook_url'], array('attributes'=> array('class' => 'btn btn-primary', 'target' => '_blank')));
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
                  <p><input class="form-control" value="$url" onclick="this.select()"></p>
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
  $vars['target_environments'];

  // Prepare environments output
  foreach ($vars['node']->project->environments as &$environment) {

    // Render each environment.
    $vars['environments'][] = theme('environment', array(
      'environment' => $environment,
      'project' => $vars['node']->project,
    ));
  }

  // Warnings & Errors
  // If environment-specific deploy hooks is not allowed and there are no default deploy hooks, warn the user
  // that they will have to manually run updates.
  if (!$vars['node']->project->settings->deploy['allow_environment_deploy_config'] && count(array_filter($vars['node']->project->settings->deploy['default_hooks'])) == 0) {
    $vars['project_messages'][] = array(
      'message' => t('No deploy hooks are configured for this project. If new code is deployed, you will have to run update.php manually. Check your !link.', array(
        '!link' => l(t('Project Settings'),"node/{$vars['node']->nid}/edit"),
      )),
      'icon' => '<i class="fa fa-exclamation-triangle"></i>',
      'type' => 'warning',
    );
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