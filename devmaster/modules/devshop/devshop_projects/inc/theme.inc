<?php

use Symfony\Component\Yaml\Parser;
use Symfony\Component\Yaml\Dumper;
use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;
use SensioLabs\AnsiConverter\AnsiToHtmlConverter;
use SensioLabs\AnsiConverter\Theme\Theme;
use SensioLabs\AnsiConverter\Theme\SolarizedTheme;
use SensioLabs\AnsiConverter\Theme\SolarizedXTermTheme;

/**
 * Implements hook_theme()
 */
function devshop_projects_theme() {
  return array(
    'devshop_projects_settings_form' => array(
      'variables' => array(
        'form' => NULL,
      ),
    ),
    'devshop_projects_create_settings_form' => array(
      'render element' => 'form',
    ),
    'devshop_project_nav' => array(
      'variables' => array(
        'node' => NULL,
        'settings_active' => NULL,
        'logs_active' => NULL,
        'branches_class' => NULL,
      ),
      'template' => 'project-nav',
    ),
    'devshop_project_add_status' => array(
      'variables' => array(
        'project' => NULL,
      ),
      'template' => 'project-add',
    ),
    'devshop_ascii' => array(
      'variables' => array(
        'output' => '',
      ),
    ),
    'devshop_task' => array(
      'variables' => array(
          'task' => NULL,
      ),
      'template' => 'task',
    ),
    'devshop_add_environment_options' => array(
      'render element' => 'form',
      'template' => 'add-environment-options',
      'path' => drupal_get_path('module', 'devshop_projects') . '/includes',
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
    foreach (element_children($form[$env_name]['settings']) as $setting) {
      if (!isset($header[$setting])) {
        $header[$setting] = $form[$env_name]['settings'][$setting]['#title'];
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';
      $row[] = drupal_render($form[$env_name]['settings'][$setting]);
    }
    $rows[] = $row;
  }
  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('project-environments-table'))));
  return $output;
}

/**
 * Theme function for create environments settings
 * @TODO: Fold into theme_devshop_projects_settings_form()
 */
function theme_devshop_projects_create_settings_form($vars) {

  $form = $vars['form'];
  $rows = array();
  $header = array();
  foreach (element_children($form) as $env_name) {
    $row = array();
    $header['name'] = 'Name';
    $header['git_ref'] = t('Branch/Tag');

    $name_element = $form[$env_name]['name'];
    $git_ref_element = $form[$env_name]['git_ref'];
    $settings_element = $form[$env_name]['settings'];

    $row[] = drupal_render($name_element);
    $row[] = drupal_render($git_ref_element);

    foreach (element_children($settings_element) as $setting) {
      if (!isset($header[$setting])) {
        $header[$setting] = isset($form[$env_name]['settings'][$setting]['#title']) ? $form[$env_name]['settings'][$setting]['#title'] : '';
      }
      $form[$env_name]['settings'][$setting]['#title'] = '';

      $element = $form[$env_name]['settings'][$setting];
      $row[] = drupal_render($element);
    }
    $rows[] = $row;
  }
  $output = theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array(
          'class' => array('table', 'project-environments-table'),
      )
  ));

  return $output;
}

/**
 * Preprocess page
 */
function devshop_projects_preprocess_node(&$vars) {

  $node = $vars['node'];

  // On project node edit page
  if (isset($vars['node']) && $vars['node']->type == 'project') {

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
  if (isset($vars['node']) && $vars['node']->type == 'task' && ($vars['page'] || current_path() == "hosting/task/{$node->nid}")){
    // Prepare task icon, class, and label.
    $icon = devshop_task_status_icon($vars['node']->task_status);
    $vars['task_label'] = "<i class='fa fa-$icon'></i> " . _hosting_parse_error_code($vars['node']->task_status);
    $vars['task_label_class'] = devshop_task_status_class($vars['node']->task_status);
    $vars['task_icon'] = devshop_task_status_icon($vars['node']->task_status);

    // Load ref node
    $ref_node = node_load($vars['rid']);
    if ($ref_node->type == 'site') {
      $vars['site_url'] = l($ref_node->environment->url, $ref_node->environment->url);
    }

    // Add a "Retry" button, unless it's a clone task.  Retrying a clone can be bad if an old clone task is restarted, it uses the latest task arguments, not the one you are viewing.
    if ($vars['node']->task_type != 'clone' && user_access('retry failed tasks') && (
        $vars['node']->task_status == HOSTING_TASK_ERROR ||
        $vars['node']->task_status == HOSTING_TASK_WARNING ||
        $vars['node']->task_status == HOSTING_TASK_SUCCESS
      )) {

      // If "install" task, change name and add "force" task arg.
      $form = drupal_get_form('hosting_task_retry_form', $vars['node']->nid);
      $vars['retry'] = $form;

      if ($vars['node']->task_type == 'install' && !$vars['node']->environment->settings->locked) {
        $vars['retry'] = $form;
        $vars['retry']['retry']['#value'] = t('Reinstall');
      }
      elseif ($vars['node']->task_type == 'install' && $vars['node']->environment->settings->locked) {
        $vars['retry'] = array(
          '#children' => t('Site is protected.'),
          '#type' => 'container',
          '#attributes' => array(
            'class' => array('well well-sm')
          ),
        );
        $settings_link = "node/{$vars['node']->rid}/edit";
        if (drupal_valid_path($settings_link)) {
          $vars['retry']['#children'] .= ' ' . l(t('Environment Settings'), $settings_link);
        }
      }
      elseif ($vars['node']->task_type != 'install' && $vars['node']->task_type != 'delete' || ($vars['node']->task_type != 'delete' && $vars['node']->task_status == HOSTING_TASK_ERROR)) {
        $vars['retry'] = $form;
      }

      // Change the name of the form button to "Run Again" if the task ended ok.
      if ($vars['node']->task_status == HOSTING_TASK_WARNING || $vars['node']->task_status == HOSTING_TASK_SUCCESS) {
        $vars['retry']['retry']['#value'] = t('Run Again');
      }
    }

    // Show duration
    if ($vars['node']->task_status == HOSTING_TASK_QUEUED) {
      $vars['duration'] = t('Queued for %time', array('%time' => format_interval(REQUEST_TIME - $vars['node']->changed)));
      $vars['date'] = date('D M j Y', $vars['node']->changed);
      $vars['executed'] = '';
    }
    else {

      if ($vars['node']->task_status == HOSTING_TASK_PROCESSING) {
        $vars['duration'] = format_interval(REQUEST_TIME - $vars['node']->executed, 1);
        $vars['executed'] = '';
      }
      else {
        $vars['duration'] = format_interval($vars['node']->delta, 1);
        $vars['executed'] = format_interval(REQUEST_TIME - $vars['node']->executed) . ' ' . t('ago');
      }
      $vars['date'] = date('D M j Y', $vars['node']->executed);
    }

    // Load Logs
    $messages = devshop_task_get_messages($vars['node']);
    $vars['messages'] = implode("\n", $messages);
    switch ($vars['task_type']) {
      case 'deploy':
        $vars['type'] = 'Deploy';
        break;
      case 'verify':
      case 'test':
      case 'sync':
      case 'install':
      default:
        $vars['type'] = ucfirst($vars['task_type']);
        break;
    }

    // Add "Follow Logs" button.
    if ($vars['node']->task_status == HOSTING_TASK_PROCESSING || $vars['node']->task_status == HOSTING_TASK_QUEUED ) {
      $vars['follow_checkbox'] = array(
        '#type' => 'markup',
        '#markup' => '<label for="follow" type="button" class="follow-checkbox btn btn-link"><input type="checkbox" id="follow"> ' . t('Scroll Logs') . '</label>',
      );

      $vars['follow_checkbox'] = drupal_render($vars['follow_checkbox']);
    }

    // Running indicator
    if ($vars['task_status'] == HOSTING_TASK_QUEUED ||$vars['task_status'] == HOSTING_TASK_PROCESSING) {
      $vars['is_active'] = 'active';

      if ($vars['task_status'] == HOSTING_TASK_PROCESSING) {
        $vars['is_running'] = 'fa-spin';
        $vars['running_label'] = t('Processing...');
      }
      else {
        $vars['running_label'] = t('Queued');
      }
    }
  }
}

/**
 * Helper to retrieve helpful messages from aegir task logs.
 * @param $task_node
 * @return array
 */
function devshop_task_get_messages($task_node) {
  $messages = array();
  $outputs = array();

  $result = db_query("
    SELECT message, type, timestamp FROM {hosting_task_log}
      WHERE nid = :nid
        AND vid = :vid
        AND (type LIKE :devshop OR type = :error OR type = :warning)
      ORDER BY lid", array(
    ':nid' => $task_node->nid,
    ':vid' => $task_node->vid,
    ':devshop' => 'p_%',
    ':error' => 'error',
    ':warning' => 'warning',
//    ':success' => 'success',
//    ':ok' => 'ok',
  ));
  $process_logs = FALSE;

  foreach ($result as $record) {

    if (strpos($record->type, 'p_') === 0) {
      $process_logs = TRUE;
    }

    if ($record->type == 'p_command') {
      if (empty($record->message)) {
        continue;
      }
      $command = $record->message;
      $outputs[$command] = array(
        'status' => 'default',
        'icon' => 'gear',
        'output' => '',
      );

      if ($task_node->task_status == HOSTING_TASK_PROCESSING) {
        $outputs[$command]['icon'] .= ' fa-spin';
      }
      continue;
    } elseif ($record->type == 'p_info') {
      $outputs[$command]['output'] .= $record->message;
      $outputs[$command]['status'] = 'default';
      $outputs[$command]['icon'] = 'gear fa-spin';
    } elseif ($record->type == 'p_ok') {
      $outputs[$command]['output'] .= $record->message;
      $outputs[$command]['status'] = 'success';
      $outputs[$command]['icon'] = 'check';
    } elseif ($record->type == 'p_error') {
      $outputs[$command]['status'] = 'danger';
      $outputs[$command]['icon'] = 'exclamation-circle';
      $outputs[$command]['output'] = isset($outputs[$command]['output']) ? $outputs[$command]['output'] .= $record->message : $record->message;
    } elseif ($record->type == 'warning') {
      $command = 'warning_' . count($outputs);
      $outputs[$command]['status'] = 'warning';
      $outputs[$command]['icon'] = 'exclamation-triangle';
      $outputs[$command]['output'] = isset($outputs[$command]['output']) ? $outputs[$command]['output'] .= $record->message : $record->message;
      $outputs[$command]['command'] = 'warning';

    }

    // If error is the one caused by "drush_set_error", don't bother displaying because the process one works well on it's own.
    elseif ($process_logs && $record->type == 'error' && $record->message == 'PROVISION_PROCESS_ERROR') {
      continue;

    } elseif ($record->type == 'error') {
      $command = 'error_' . count($outputs);
      $outputs[$command]['status'] = 'danger';
      $outputs[$command]['icon'] = 'exclamation-circle';
      $outputs[$command]['output'] = $record->message;

      // @TODO: Here, we can check for obscure aegir errors and write our own.
      // devshop_check_errors($outputs[$command]['output']);
      $outputs[$command]['command'] = 'error';
    } elseif ($record->type == 'p_log') {
      $command = 'log_' . count($outputs);
      $outputs[$command]['status'] = '';
      $outputs[$command]['icon'] = '';
      $outputs[$command]['command'] = $record->message;
      $outputs[$command]['output'] = '';
    } elseif ($record->type == 'p_notice') {
      $command = 'notice_' . count($outputs);
      $outputs[$command]['status'] = '';
      $outputs[$command]['icon'] = '';
      $outputs[$command]['command'] = '';
      $outputs[$command]['output'] = $record->message;
    }
    // Let's show some success messages. There are way too many for verify and install tasks.
    elseif (($record->type == 'success' || $record->type == 'ok') && $task_node->task_type != 'verify' && $task_node->task_type != 'install') {
      $command = 'success_' . count($outputs);
      $outputs[$command]['output'] .= $record->message;
      $outputs[$command]['status'] = 'success';
      $outputs[$command]['icon'] = 'check';
      $outputs[$command]['command'] = 'drush';
    }
    $outputs[$command]['type'] = $record->type;
  }

  foreach ($outputs as $command => $data) {
    $status = $data['status'];
    $icon = $data['icon'];

    // Allow $outputs array to override the displayed command.
    if (isset($data['command'])) {
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
    $output = theme('devshop_ascii', array('output' => $data['output']));

    if (!empty($output)) {
      $body = "<div class='panel-body panel-type-{$data['type']} panel-ansii'>$output</div>";
    }
    else {
      $body = '';
    }

    if (!empty($command)) {
      $header = "<div class='panel-heading'><i class='fa fa-$icon pull-left'></i> $command</div>";
    }
    else {
      $header = '';
    }

    if (empty($status)) {
      $status = 'default';
    }

    $messages[] = <<<HTML
<div class="panel panel-$status devshop-command">
  $header
  $body
</div>
HTML;
  }
 return $messages;
}

/**
 * Preprocess for project_add_status.tpl.php
 */
function template_preprocess_devshop_project_add_status(&$vars) {
  if (isset($vars['project']->settings, $vars['project']->settings->default_environment)) {
    $vars['web_server_node'] = isset($vars['project']->settings->default_environment['web_server']) ? node_load($vars['project']->settings->default_environment['web_server']) : NULL;
    $vars['db_server_node'] = isset($vars['project']->settings->default_environment['db_server']) ? node_load($vars['project']->settings->default_environment['db_server']) : NULL;
  }
}

/**
 * Preprocess for project_nav.tpl.php
 */
function template_preprocess_devshop_project_nav(&$vars) {

  global $user;
  $project = $vars['project'];
  $node = $vars['node'] = node_load($project->nid);

  // @TODO: Detect other web URLs for other git hosts.
  if ($project->git_repo_url) {
    $vars['github_url'] = $project->git_repo_url;
  }

  // Generate branches/tags lists
  $vars['branches_count'] = isset($project->settings->git['branches']) ? count($project->settings->git['branches']) : 0;
  $vars['tags_count'] = isset($project->settings->git['tags']) ? count($project->settings->git['tags']) : 0;
  $vars['branches_items'] = array();
  $vars['branches_icon'] = 'code-fork';

  if ($vars['branches_count'] == 0 && isset($node->project->verify) && isset($node->verify)) {
    // If branches are 0 and last verifying is queued...
    if ($node->project->verify->task_status == HOSTING_TASK_PROCESSING || $node->verify->task_status === HOSTING_TASK_QUEUED) {
      $vars['branches_show_label'] = TRUE;
      $vars['branches_label'] = t('Refreshing...');
      $vars['branches_class'] = 'btn-warning';
      $vars['branches_icon'] = 'gear fa-spin';
      $vars['branches_items'][] = l(t('View task log'), 'node/' . $node->project->verify->nid);

    }
    // If branches are 0 and last verifying failed...
    elseif ($node->project->verify->task_status == HOSTING_TASK_ERROR) {
      $vars['branches_show_label'] = TRUE;
      $vars['branches_label'] = t('Error');
      $vars['branches_class'] = 'btn-danger';
      $vars['branches_items'][] = t('There was a problem refreshing branches and tags.');
      $vars['branches_items'][] = l(t('View task log'), 'node/' . $node->project->verify->nid);
    }
    // If branches are 0 and last verifying has completed... This should never happen, because the task would error out.
    elseif ($node->project->verify->task_status == HOSTING_TASK_SUCCESS) {
      $vars['branches_show_label'] = TRUE;
      $vars['branches_label'] = t('No branches found!');
    }
  }
  // If there are branches... build the branch items
  else {
    $vars['branches_show_label'] = FALSE;
    $vars['branches_label'] = format_plural($vars['branches_count'], t('1 Branch'), t('!count Branches', array('!count' => $vars['branches_count'])));

    if (isset($project->settings->git['branches'])) {
      foreach ($project->settings->git['branches'] as $branch) {
        $href = isset($vars['github_url']) ? $vars['github_url'] . '/tree/' . $branch : '#';
        $vars['branches_items'][] = "<a href='$href'><i class='fa fa-code-fork'></i> $branch </a>";
      }
    }
  }


  if ($vars['tags_count']) {
//      <li class="divider"></li>

    $vars['branches_label'] .= ' &amp; ' . format_plural($vars['tags_count'], t('1 Tag'), t('!count Tags', array('!count' => $vars['tags_count'])));


    foreach ($project->settings->git['tags'] as $branch) {
      $href = isset($vars['github_url']) ? $vars['github_url'] . '/tree/' . $branch : '#';
      $vars['branches_items'][] = "<a href='$href'><i class='fa fa-tag'></i> $branch </a>";
      $vars['git_refs'][] = $branch;
    }
  }

  $vars['dashboard_link'] = l('<i class="fa fa-cubes"></i> ' . t('Dashboard'), "node/$project->nid", array('html' => TRUE));

  if (arg(2) == 'edit') {
    $vars['settings_active'] = 'active';
  }
  if (arg(2) == 'logs') {
    $vars['logs_active'] = 'active';
  }

  // Add "refresh branches" link if project is manual deploy mode or is missing a webhook
  if (isset($project->settings->deploy) && $project->settings->deploy['method'] == 'manual' || empty($project->settings->deploy['last_webhook'])) {
    $link_refresh = l(t('Refresh branches'), 'hosting_confirm/' . $node->nid . '/project_verify', array('attributes' => array('class' => array('refresh-link')), 'query' => array('token' => drupal_get_token($user->uid))));
    array_unshift($vars['branches_items'], $link_refresh);
  }

  // Scrub git urls
  $vars['safe_git_url'] = scrub_repo_url($project->git_url);
}

/**
 * Output pretty console themed ASCII.
 *
 * Usage:
 *   theme('devshop_ascii', $string);
 */
function theme_devshop_ascii($vars) {
  $string = $vars['output'];
  if (empty($string)) {
    return $string;
  }

  // Prepare ASCII Converter
  $theme = new SolarizedXTermTheme();
  $styles = $theme->asCss();
  $styles .= ".ansi_box { overflow: auto; padding: 10px 15px; font-family: monospace; }";

  drupal_add_html_head("<style>$styles</style>");
  $converter = new AnsiToHtmlConverter($theme);
  $output = $converter->convert($string);
  
  // Make URLs clickable:
  $filter = new stdClass();
  $filter->settings = [
    'filter_url_length' => 1000,
  ];
  $output = _filter_url($output, $filter);
  
  // Change linebreaks to <br> tags.
  $output = strtr($output, array(
    '\n' => '<br />',
  ));

  return <<<HTML
  <pre class="ansi_color_bg_black ansi_color_fg_white ansi_box">$output</pre>
HTML;

}

/**
 * Replaces a http git URL with an embedded password the *****
 *
 * @param $project_git_url
 * @return string
 */
function scrub_repo_url($project_git_url){

  $url = parse_url($project_git_url);
  if (isset($url['pass'])) {
    return strtr($project_git_url, array(
      $url['pass'] => '****',
    ));
  }
  else {
    return $project_git_url;
  }
}
