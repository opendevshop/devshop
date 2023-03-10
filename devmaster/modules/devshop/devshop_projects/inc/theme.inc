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
    'environment' => array(
      'arguments' => array(
        'environment' => NULL,
        'project' => NULL,
        'page' => FALSE,
      ),
      'template' => 'environment',
      'path' => drupal_get_path('module', 'devshop_projects') . '/templates',
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
  global $user;
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

      if ($vars['node']->task_type == 'install' && !$vars['node']->environment->settings->locked && ($vars['node']->task_status == HOSTING_TASK_SUCCESS || $vars['node']->task_status == HOSTING_TASK_WARNING)) {
          $vars['retry'] = [
            '#type' => 'link',
            '#title' => t('Reinstall'),
            '#href' => "hosting_confirm/{$vars['node']->rid}/site_install",
            '#options' => array(
              'query' => array(
                'token' => drupal_get_token($user->uid)
              ),
            ),
          ];
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
      elseif ($vars['node']->task_status == HOSTING_TASK_WARNING || $vars['node']->task_status == HOSTING_TASK_SUCCESS) {
          // Change the name of the form button to "Run Again" if the task ended ok.
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

  global $user;
  if ($vars['node']->type == 'project') {
    devshop_projects_preprocess_node_project($vars);
  }
  elseif ($vars['node']->type == 'task') {
    devshop_projects_preprocess_node_task($vars);
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
function devshop_projects_preprocess_node_task(&$vars) {
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
function devshop_projects_preprocess_node_project(&$vars){
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
  if (!empty($project->settings->deploy['webhooks']) && $project->settings->deploy['last_webhook']){
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
  else {
    $environments = $vars['node']->project->environments;
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
  if (isset($project->settings->deploy['method']) && $project->settings->deploy['method'] == 'manual' || empty($project->settings->deploy['last_webhook'])) {
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

/**
 * Preprocessor for environment template.
 */
function devshop_projects_preprocess_environment(&$vars) {
  $environment = &$vars['environment'];
  $project = &$vars['project'];

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

  foreach ($environment->tasks_list as $i => $task) {
    if ($i == 0) continue;

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
  elseif ($environment->site_status == HOSTING_SITE_DELETED) {
    $vars['warnings'][] = array(
      'text' => t('Site Destroyed'),
      'type' => 'info',
    );
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
  $environment->git_last_timestamp = date('N', $environment->git_last);
  $environment->git_last_ago = t('!ago ago', array(
    '!ago' => format_interval(time() - $environment->git_last , 1)
  ));
  
  /**
   * @TODO: Change how this works. Git can no longer run commands on a repo with another user without marking "safe.directory" for every repo.
   *
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
//    $environment->git_ref = trim(str_replace('refs/heads/', '', shell_exec("cd {$environment->repo_path}; git describe --tags --exact-match 2> /dev/null || git symbolic-ref -q HEAD 2> /dev/null")));

    $environment->git_ref_type = !empty($project->settings->git['refs'][$environment->git_ref])
      ? $project->settings->git['refs'][$environment->git_ref]
      : 'branch';

    // If the git sha for stored branch are the same, but the type is different, detect if HEAD is detached so we know if this is on a branch or a tag.
    if ($stored_git_sha == $environment->git_sha && $stored_git_ref_type != $environment->git_ref_type) {
      $git_status = shell_exec("cd {$environment->repo_path}; git status");
      if (strpos($git_status, 'On branch ') === 0) {
        $environment->git_ref_type = 'branch';
//        $environment->git_ref = $environment->git_ref_stored;
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
   * */

  // Look for alternative git URL
  if ($environment->git_url != $project->git_url) {
    $vars['git_origin'] = $environment->git_url;
  }
}

/**
 * Implements hook_preprocess_page()
 * @param $vars
 */
function devshop_projects_preprocess_page(&$vars){
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
    $vars['content_column_class_devshop'] = ' class="col-sm-9"';
  }
  else {
    $vars['content_column_class_devshop'] = ' class="col-sm-12"';
  }

  if (user_access('access task logs')){
    $vars['tasks'] = devshop_render_tasks();
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
  if (!empty($vars['page']['sidebar_first'])) {
    $vars['sidebar_first_rendered'] = render($vars['page']['sidebar_first']);
  }
  else {
    $vars['sidebar_first_rendered'] = '';
  }

  // Indicate the project is inactive.
  if (isset($project_node) && isset($project_node->nid) && (int) $project_node->status == NODE_NOT_PUBLISHED) {
    $vars['subtitle'] .= ' <small>' . t('Inactive Project') .  '</small>';
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
function devshop_render_tasks($tasks = NULL, $class = '', $actions = array(), $float = 'left'){
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
