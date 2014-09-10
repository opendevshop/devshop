<?php

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
function boots_render_tasks($tasks = NULL, $class = ''){

  if (is_null($tasks)){
    // Tasks
    $tasks = hosting_get_tasks(null, null, 100);
  }

  // Get active or queued
  $tasks_count = 0;
  foreach ($tasks as $task){
    if ($task->task_status == HOSTING_TASK_QUEUED || $task->task_status == HOSTING_TASK_PROCESSING){
      $tasks_count++;
    }
  }

  if ($tasks_count > 0) {
    $task_class = 'fa-spin active-task-gear';
  }

  if (!empty($tasks)) {
    $items = array();

    foreach ($tasks as $task) {

      switch ($task->task_status){
        case HOSTING_TASK_SUCCESS:
          $icon = 'check text-success';
          $item_class = 'bg-success';
          break;

        case HOSTING_TASK_ERROR;
          $icon = 'exclamation-circle text-danger';
          $item_class = 'bg-danger';
          break;

        case HOSTING_TASK_PROCESSING;
        case HOSTING_TASK_QUEUED;
          $icon = 'cog fa-spin text-info';
          $item_class = 'bg-info';
          break;


      }
      $text = '<i class="fa fa-' . $icon . '"></i> ';
      $text .= $task->title;
      $text .= ' <small>' . format_interval(time() - $task->changed) .' '. t('ago') . '</small>';

      $items[] =  array(
        'data' => l($text, 'node/' . $task->rid, array(
          'html' => TRUE,
        )),
        'class' => $item_class,
      );
    }
  }
  else {
    $items[] = t('No Active Tasks');
  }
  $items[] = array(
    'class' => 'divider',
  );

  $text = '<i class="fa fa-list-alt"></i> '. t('Task Logs');
  $items[] = l($text, 'hosting/queues/tasks', array('html' => TRUE));

  $tasks = theme('item_list', $items, '', 'ul', array('class' => 'dropdown-menu dropdown-menu-right', 'role' => 'menu'));

  return <<<HTML
    <a href="#" class="dropdown-toggle $class" data-toggle="dropdown">
      <i class="fa fa-gear $task_class"></i>
        $tasks_count
      <span class="caret"></span>
    </a>
    $tasks
HTML;

}

/**
 * Implements hook_preprocess_page()
 * @param $vars
 */
function boots_preprocess_page(&$vars){

  $vars['tasks'] = boots_render_tasks();

  if ($vars['node']) {

    // Removing conflicting scripts.
    // Not sure how this actually works.  Drupal 6 is fun!
    // Thanks, http://drupal.stackexchange.com/questions/5076/remove-every-javascript-except-own-theme-scripts
    drupal_add_js(path_to_theme(). '/js/bootstrap.min.js', 'theme');
    $js = drupal_add_js();
    unset($js['core']);
    unset($js['module']);
    $vars['scripts'] = $js;

    // Set subtitle
    if ($vars['node']->type == 'project'){
      $vars['subtitle'] = t('Project');

      unset($vars['tabs']);

      $vars['title'] = l($vars['title'], "node/" . $vars['node']->nid);
    }
  }
}


/**
 *
 * @param $vars
 */
function boots_preprocess_node(&$vars){
  global $user;
  if ($vars['node'] && $vars['node']->type == 'project') {

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
        $href = url('node/' . $node->nid . '/project_devshop-deploy', array(
          'query' =>array(
            'git_ref' => $ref,
            'environment' => 'ENV',
          )
        ));
        $icon = $type == 'tag'? 'tag': 'code-fork';

        $vars['git_refs'][] = "<a href='$href'>
          <i class='fa fa-$icon'></i>
          $ref
        </a>";
      }
    }

    // Get available servers
    $vars['web_servers'] = hosting_get_servers('http');
    $vars['db_servers'] = hosting_get_servers('db');


    // Get Drush aliases
    $vars['drush_aliases'] = 'COMING SOON';

  }
}

/**
 * Override for item_list
 */
function boots_item_list($items = array(), $title = NULL, $type = 'ul', $attributes = NULL) {
  $output = '';
  if (!empty($title)) {
    $output .= '<h3>' . $title . '</h3>';
  }

  if (!empty($items)) {
    $output .= "<$type" . drupal_attributes($attributes) . '>';
    $num_items = count($items);
    foreach ($items as $i => $item) {
      $attributes = array();
      $children = array();
      if (is_array($item)) {
        foreach ($item as $key => $value) {
          if ($key == 'data') {
            $data = $value;
          }
          elseif ($key == 'children') {
            $children = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $data = $item;
      }
      if (count($children) > 0) {
        $data .= theme_item_list($children, NULL, $type, $attributes); // Render nested list
      }
      if ($i == 0) {
        $attributes['class'] = empty($attributes['class']) ? 'first' : ($attributes['class'] . ' first');
      }
      if ($i == $num_items - 1) {
        $attributes['class'] = empty($attributes['class']) ? 'last' : ($attributes['class'] . ' last');
      }
      $output .= '<li' . drupal_attributes($attributes) . '>' . $data . "</li>\n";
    }
    $output .= "</$type>";
  }
  return $output;
}