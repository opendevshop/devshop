<?php

function boots_preprocess_page(&$vars){

  // Tasks
  $tasks = hosting_get_tasks('task_status', HOSTING_TASK_PROCESSING);
  $vars['tasks_count'] = count($tasks);

  $tasks = hosting_get_tasks('task_status');

  if ($vars['tasks_count'] > 0) {
    $vars['task_class'] = 'fa-spin active-task-gear';
  }

  if (!empty($tasks)) {
    $items = array();

    foreach ($tasks as $task) {
      $item = l($task->title, 'node/' . $task->nid);

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

  $vars['tasks'] = theme('item_list', $items, '', 'ul', array('class' => 'dropdown-menu', 'role' => 'menu'));

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

    // @TODO: Detect other web URLs for other git hosts.
    if (strpos($project->git_url, 'github.com') !== FALSE) {
      $url = str_replace('git@github.com:', 'http://github.com/', $project->git_url);
      $vars['github_url'] = $url;
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
            'environment' => '{ENV}',
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