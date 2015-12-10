<?php

use Symfony\Component\Yaml\Parser;
use Symfony\Component\Yaml\Dumper;
use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;
use SensioLabs\AnsiConverter\AnsiToHtmlConverter;
use SensioLabs\AnsiConverter\Theme\SolarizedTheme;
use SensioLabs\AnsiConverter\Theme\SolarizedXTermTheme;

/**
 * Implements hook_hosting_theme()
 */
function devshop_projects_theme(){
  return array(
    'devshop_projects_settings_form' => array(
      'arguments' => array(
        'form' => NULL,
      ),
    ),
    'devshop_projects_create_settings_form' => array(
      'arguments' => array(
        'form' => NULL,
      ),
    ),
    'devshop_project_nav' => array(
      'arguments' => array(
        'node' => NULL,
      ),
      'template' => 'project-nav',
    ),
    'devshop_project_add_status' => array(
      'arguments' => array(
        'project' => NULL,
      ),
      'template' => 'project-add',
    ),
  );
}

/**
 * Theme function for environments settings
 */
function theme_devshop_projects_settings_form($form) {
  $rows = array();
  $header = array();
  $header[] = t('Environment');
  foreach (element_children($form) as $env_name) {
    $row = array();
    $row[] = $env_name . drupal_render($form[$env_name]['git_ref']);
    foreach(element_children($form[$env_name]['settings']) as $setting){
      if (!isset($header[$setting])){
        $header[$setting] = $form[$env_name]['settings'][$setting]['#title'];
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name]['settings'][$setting]);
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('id' => 'project-settings-table'));
  return $output;
}

/**
 * Theme function for create environments settings
 * @TODO: Fold into theme_devshop_projects_settings_form()
 */
function theme_devshop_projects_create_settings_form($form) {
  $rows = array();
  $header = array();
  foreach (element_children($form) as $env_name) {
    $row = array();
    $header['name'] = 'Name';
    $header['git_ref'] = t('Branch/Tag');
    $row[] = drupal_render($form[$env_name]['name']);
    $row[] = drupal_render($form[$env_name]['git_ref']);

    foreach(element_children($form[$env_name]['settings']) as $setting){
      if (!isset($header[$setting])){
        $header[$setting] = $form[$env_name]['settings'][$setting]['#title'];
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name]['settings'][$setting]);
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('id' => 'project-settings-table', 'class' => 'table'));
  $output .= '<p>'. t('Create as many new environments as you would like. For example: "dev", "test", and "live". You can create more later on if needed.') .'</p>';

  return $output; 
}

/**
 * Preprocess page
 */
function devshop_projects_preprocess_node(&$vars){

  // On project node edit page
  if (isset($vars['node']) && $vars['node']->type == 'project'){

    $vars['drush_aliases'] = devshop_project_aliases($vars['node']->project);
    $vars['aliases_url'] = url("node/{$vars['node']->nid}/aliases");

    if (module_exists('aegir_ssh') && user_access('manage own SSH public keys')) {
      global $user;
      $vars['access_note'] = t('NOTE: To access these environments with drush remotely, make sure you have uploaded your public SSH key under !link.', array(
        '!link' => l('My Account > SSH Keys', "user/$user->uid/ssh-keys"),
      ));
    }
    else {
      $vars['access_note'] = t('NOTE: To access these environments with drush remotely, ask an administrator to add your public SSH key to the file <code>/var/aegir/.ssh/authorized_keys</code>.');
    }
  }

  // On task node page
  if (isset($vars['node']) && $vars['node']->type == 'task' && $vars['page']){
    $vars['content'] = '';

    $log_type = '';
    switch ($vars['node']->task_status) {
      case HOSTING_TASK_SUCCESS:
        $vars['task_label'] = t('Success');
        $vars['task_label_class'] = 'success';
        break;
      case HOSTING_TASK_ERROR:
        $vars['task_label'] = t('Error');
        $vars['task_label_class'] = 'danger';
        $log_type = 'error';
        break;
      case HOSTING_TASK_WARNING:
        $vars['task_label'] = t('Success (with warning)');
        $vars['task_label_class'] = 'warning';
        $log_type = 'warning';
        break;
      case HOSTING_TASK_PROCESSING:
        $vars['task_label'] = t('Processing') . ' <i class="fa fa-gear fa-spin"></i>';
        $vars['task_label_class'] = 'info';
        break;
      case HOSTING_TASK_QUEUED:
        $vars['task_label'] = t('Queued');
        $vars['task_label_class'] = 'info';
        break;
    }

    // Load ref node
    $ref_node = node_load($vars['rid']);
    if ($ref_node->type == 'site') {
      $vars['site_url'] = l($ref_node->environment->url, $ref_node->environment->url);
    }

    if (user_access('retry failed tasks') && ($vars['node']->task_status == HOSTING_TASK_ERROR)) {
      $vars['retry'] = drupal_get_form('hosting_task_retry_form', $vars['node']->nid);
    }

    // Show duration
    if ($vars['node']->task_status == HOSTING_TASK_QUEUED) {
      $vars['duration'] = 'Queued';
      $vars['date'] = date('D M j Y', $vars['node']->changed);
      $vars['executed'] = t('Queued') . ' ' . format_interval(time() - $vars['node']->changed) . ' ' . t('ago');
    }
    else {

      if ($vars['node']->task_status == HOSTING_TASK_PROCESSING) {
        $vars['duration'] = format_interval(time() - $vars['node']->executed, 1);
      }
      else {
        $vars['duration'] = format_interval($vars['node']->delta, 1);
      }
      $vars['date'] = date('D M j Y', $vars['node']->executed);
      $vars['executed'] = format_interval(time() - $vars['node']->executed) . ' ' . t('ago');
    }

    // Load Logs
    $ref = node_load($vars['rid']);
    if (isset($ref->project) && !empty($ref->project)) {
      $messages = array();
      $converter = new AnsiToHtmlConverter(null, false);

      $query = db_query("SELECT message, type FROM {hosting_task_log} WHERE vid = %d AND (type LIKE 'devshop_%' OR type = 'error' OR type = 'warning') ORDER BY vid, lid", $vars['node']->vid);
      while ($results = db_fetch_object($query)) {

        if ($results->type == 'devshop_command') {
          if (empty($results->message)) {
            continue;
          }
          $command = $results->message;
          $outputs[$command] = array(
            'status' => 'default',
            'icon' => 'gear',
            'output' => '',
          );

          if ($vars['node']->task_status == HOSTING_TASK_PROCESSING) {
            $outputs[$command]['icon'] .= ' fa-spin';
          }
          continue;
        } elseif ($results->type == 'devshop_ok') {
          $outputs[$command]['output'] .= $results->message;
          $outputs[$command]['status'] = 'success';
          $outputs[$command]['icon'] = 'check';
        } elseif ($results->type == 'devshop_error') {
          $outputs[$command]['status'] = 'danger';
          $outputs[$command]['icon'] = 'exclamation-circle';
          $outputs[$command]['output'] .= $results->message;
        } elseif ($results->type == 'warning') {
          $command = 'warning_' . count($outputs);
          $outputs[$command]['status'] = 'warning';
          $outputs[$command]['icon'] = 'exclamation-triangle';
          $outputs[$command]['output'] .= $results->message;
          $outputs[$command]['command'] = 'warning';
        } elseif ($results->type == 'error') {
          $command = 'error_' . count($outputs);
          $outputs[$command]['status'] = 'danger';
          $outputs[$command]['icon'] = 'exclamation-circle';
          $outputs[$command]['output'] .= $results->message;
          $outputs[$command]['command'] = 'error';
        }
      }

      foreach ($outputs as $command => $data) {
        $status = $data['status'];
        $icon = $data['icon'];

        // Allow $outputs array to override the displayed command.
        if (!empty($data['command'])) {
          $command = $data['command'];
        }

        // If $command has brackets, convert to span.
        if (strpos($command, '[') !== FALSE && strpos($command, ']') !== FALSE) {
          $command = strtr($command, array(
            '[' => '<div class="text-muted small">',
            ']' => '</div>',
          ));
        }

        // Convert ASCII to HTML
        $output = $converter->convert($data['output']);
        $messages[] = <<<HTML
<div class="panel panel-$status devshop-command">
  <div class="panel-heading"><i class="fa fa-$icon pull-left"></i> $command</div>
  <div class="panel-body">
     <pre>$output</pre>
  </div>
</div>
HTML;
      }

      if (count($messages)) {
        $vars['log_message'] = '';
        $messages = implode("\n", $messages);
        $content = $vars['content'];

        switch ($vars['task_type']) {
          case 'devshop-deploy':
            $type = 'Deploy';
            break;
          case 'verify':
          case 'test':
          case 'sync':
          case 'install':
          default:
            $type = ucfirst($vars['task_type']);
            break;
        }

        $vars['content'] = <<<HTML
  <h3>$type</h3>

  $messages
  $content
HTML;

      }
    }

  }
}

/**
 * Preprocess page
 */
function devshop_projects_preprocess_page(&$vars){
  if (arg(0) == 'projects' && arg(1) == 'add') {
    ctools_include('object-cache');
    $project = ctools_object_cache_get('project', NULL);
    $vars['left'] = theme('devshop_project_add_status', $project);
  }
}


/**
 * Preprocess for project_add_status.tpl.php
 */
function template_preprocess_devshop_project_add_status(&$vars) {

  $vars['web_server_node'] = node_load($vars['project']->settings->default_environment['web_server']);
  $vars['db_server_node'] = node_load($vars['project']->settings->default_environment['db_server']);
}

/**
 * Preprocess for project_nav.tpl.php
 */
function template_preprocess_devshop_project_nav(&$vars){

  global $user;
  $node = $vars['node'];
  $project = $vars['project'] = $vars['node']->project;

  // @TODO: Detect other web URLs for other git hosts.
  if ($project->git_repo_url) {
    $vars['github_url'] = $project->git_repo_url;
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
    }
    // If branches are 0 and last verifying has completed... This should never happen, because the task would error out.
    elseif ($node->verify->task_status == HOSTING_TASK_SUCCESS) {
      $vars['branches_show_label'] = TRUE;
      $vars['branches_label'] = t('No branches found!');
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

  $vars['dashboard_link'] = l('<i class="fa fa-cubes"></i> ' . t('Dashboard'), "node/$project->nid", array('html' => TRUE));

  if (node_access('update', $node)){
    $vars['settings_link'] = l('<i class="fa fa-sliders"></i> ' . t('Settings'), "node/$node->nid/edit", array('html' => TRUE));
  }

  if (arg(2) == 'edit') {
    $vars['settings_active'] = 'active';
  }
  if (arg(2) == 'logs') {
    $vars['logs_active'] = 'active';
  }

  // Add "refresh branches" link if project is manual deploy mode or is missing a webhook
  if ($project->settings->deploy['method'] == 'manual' || empty($project->settings->deploy['last_webhook'])) {
    $vars['branches_items'][] = l(t('Refresh branches'), 'node/' . $node->nid . '/project_verify', array('attributes' => array('class' => 'refresh-link'), 'query' => array('token' => drupal_get_token($user->uid))));
  }
}