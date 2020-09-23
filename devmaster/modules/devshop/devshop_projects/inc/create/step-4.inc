<?php

/**********
 * STEP 4
 * Project Settings
 *********/

/**
 * STEP 4: Form
 */
function devshop_project_create_step_sites($form, &$form_state) {
  $project = $form_state['project'];
  $profiles = array();
  $available_profiles = array();
  $completed = TRUE;
  $task_failed = FALSE;

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project->nid,
  );

  // Display the platforms
  $rows = array();
  $header = array(
    t('Name'),
    t('Branch'),
    t('Version'),
    t('Install Profiles'),
    t('Status')
  );
  $all_tasks_queued = TRUE;
  $all_tasks_succeeded = TRUE;
  $modals = '';
  foreach ($project->environments as $name => $environment) {

    // Get platform and latest verify task.
    $platform_nid = $environment->platform;
    $platform = node_load($platform_nid);
    $tasks = devshop_get_tasks($environment, 'verify');
    $task = array_shift($tasks['verify']);

    if (empty($task)) {
      $row = array(
        $environment->name,
        $environment->git_ref,
        t('Error: Platform node was not saved. Unable to continue.'),
        '', '',
      );
      $rows[] = $row;
      continue;
    }

    // Build a table.
    $row = array();
    $row['name'] = $name;
    $row['branch'] = $environment->git_ref;
    $row['version'] = t('...');

    // If platform verified successfully:
    if ($task->task_status == HOSTING_TASK_SUCCESS || $task->task_status == HOSTING_TASK_WARNING) {

      // It's not really ready until we get a version.
      if (empty($platform->release->version)) {
        $completed = FALSE;
        $row['version'] = '...';
        $row['profiles'] = '...';
      }
      else {
        $row['version'] = $platform->release->version;
      }

      // Collect install profiles
      $profiles_shortnames = hosting_get_profiles($platform->nid, 'short_name');

      if (is_array($profiles_shortnames) && !empty($profiles_shortnames)) {
        $profiles[$name] = array_combine($profiles_shortnames, (array) hosting_get_profiles($platform->nid));
        $row['profiles'] = implode(', ', $profiles[$name]);
      }
      else {
        $profiles[$name] = array();
      }

      // If no tasks have failed, save available profiles
      if ($all_tasks_succeeded) {
        if (empty($available_profiles)) {
          $available_profiles = $profiles[$name];
        }
        else {
          $available_profiles = array_intersect_key($available_profiles, $profiles[$name]);
        }
      }
    }
    // If platform verification failed:
    elseif ($task->task_status == HOSTING_TASK_ERROR) {
      $completed = TRUE;
      $all_tasks_succeeded = FALSE;
      $available_profiles = array();

      $errors = db_query("SELECT * FROM {hosting_task_log} WHERE vid = :vid AND (type LIKE :type OR type = :error) ORDER BY vid, lid", array(':vid' => $task->vid, ':type' => 'p_error', ':error' => 'error'));

      foreach ($errors as $error) {
        if ($error->type == 'p_command' || $error->type == 'error') {
          $messages_to_user[] = $error->message;
        }
        $messages[] = $error;
      }
      $errors_rendered = theme('devshop_ascii', array('output' => implode('\n', $messages_to_user)));

      $codebase_verification_failed = t('Codebase Verification Failed');
      $close = t('Close');
      $view_logs = t('View Logs');
      $logs_url = url("node/{$task->nid}");

      $button = <<<HTML
<button type="button" class="btn btn-xs btn-danger text-muted" data-toggle="modal" data-target="#errors-modal-$name" title="View detailed errors">
        <i class="fa fa-exclamation-circle"></i>
        $codebase_verification_failed
      </button>
HTML;

      $modals .= <<<HTML

      <!-- Modal -->
      <div class="modal modal-lg fade" id="errors-modal-$name" tabindex="-1" role="dialog" aria-labelledby="errors-modal" aria-hidden="true">
        <div class="modal-dialog">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">&times;</span><span class="sr-only">Close</span></button>
              <h4 class="modal-title" id="errors-modal">$codebase_verification_failed</h4>
            </div>
            <div class="modal-body">
            $errors_rendered
            </div>
            <div class="modal-footer">
              <a href="$logs_url" class="btn btn-link pull-left">$view_logs</a>
              <button type="button" class="btn btn-default" data-dismiss="modal">$close</button>
            </div>
          </div>
        </div>
      </div>
HTML;

      $row['version'] = array(
        'data' => $button,
        'colspan' => 2,
      );
    }
    // If platform is still processing:
    elseif ($task->task_status == HOSTING_TASK_PROCESSING || $task->task_status == HOSTING_TASK_QUEUED) {
      $completed = FALSE;
      $row['version'] = "<span id='version-{$platform->nid}'>...</span>";
      $row['profiles'] = "<span id='profiles-{$platform->nid}'>...</span>";
    }

    // If a single task is not queued, $all_tasks_queued == FALSE
    if ($task->task_status != HOSTING_TASK_QUEUED) {
      $all_tasks_queued = FALSE;
    }

    // If a single task is not queued, $all_tasks_queued == FALSE
    if ($task->task_status == HOSTING_TASK_ERROR) {
      $task_failed = TRUE;
    }

    // Add hosting task status.
    $link_text = t('View Logs');
    $row['status'] = l("<span id='status-{$platform->nid}'>{$link_text}</span>", "node/{$task->nid}", array('html' => TRUE, 'attributes' => array('target' => '_blank')));

    // Store rows for display
    $rows[] = $row;
  } // end foreach platform

  // Output our table.
  $form['platforms'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array(
      'class' => array('table')
    ),
  );

  // Not completed means show all tasks are not completed (or errored)
  if (!$completed) {
    $form['#no_finish'] = TRUE;
    $note = '<p class="wait">' . t('Please wait while we clone your repo and install your Drupal code. This may take a while depending on the size of your repo and the number of Composer dependencies.') . '</p>';
    $note .= '<p class="wait"><i class="fa fa-spinner fa-spin fa-4x text-primary"></i></p>';

    $form['help'] = array(
      '#markup' => $note,
    );

    // Add code to reload the page when complete.
    devshop_form_reloader($form, 'platform');
    return $form;
  }
  elseif ($task_failed) {
    $project->no_finish = TRUE;

    $message = t('Something went wrong when trying to clone your codebase.');

    if (strpos($errors_rendered, 'Check the Publish Path of the platform') !== FALSE) {
      if (empty($project->drupal_path)) {
        $message = t("Drupal was not found in the project at the root. !link and try again.", array(
          '!link' => l(t('Change Document Root in Project Settings'), '/projects/add/settings'),
        ));
      }
      else {
        $message = t("<p>Drupal was not found in the project in the folder <code>@folder</code>.</p><p>!link and try again.</p>", array(
          '@folder' => $project->drupal_path,
          '!link' => l(t('Change Document Root in Project Settings'), '/projects/add/settings'),
        ));
      }
    }

    $form['error'] = array(
      '#prefix' => '<div class="alert alert-danger">',
      '#markup' => $message,
      '#suffix' => '</div>',
    );
    $form['errors'] = array(
      '#markup' => $errors_rendered,
    );
    $form['retry'] = array(
      '#type' => 'submit',
      '#value' => t('Retry'),
      '#prefix' => '<div class="text-center">',
      '#suffix' => '</div>',
    );
    $form['#no_finish'] = TRUE;
  }

  // If ready...
  else {

    // If no common profiles found, just set to standard.
    if (count($available_profiles) == 0) {
      $available_profiles['standard'] = 'No default profile.';
      $default_profile = 'standard';
    }

    $project->no_finish = FALSE;

    // Install Profile
    // Sensible default?
    // Lets go with standard for now... we can update later.
    if (isset($available_profiles['standard'])) {
      $default_profile = 'standard';
    }
    // If 'drupal' profile exists, it is likely drupal6!
    elseif (isset($available_profiles['drupal'])) {
      $default_profile = 'drupal';
    }

    $form['install_profile'] = array(
      '#type' => 'radios',
      '#options' => $available_profiles,
      '#title' => t('Project Install Profile'),
      '#required' => 1,
      '#description' => t('Choose the default installation profile for this project.'),
      '#default_value' => $default_profile,
    );
  }

  $form['#suffix'] = $modals;
  return $form;
}

/**
 * STEP 4: Validate
 */
function devshop_project_create_step_sites_validate(&$from, &$form_state) {
  if ($form_state['triggering_element']['#value'] == t('Retry')) {
    foreach ($form_state['project']->environments as $name => $environment) {
        hosting_add_task($environment->platform);
    }
    drupal_goto('projects/add/sites');
  }
  elseif (empty($form_state['values']['install_profile'])) {
    form_set_error('install_profile', t('You must choose an install profile. Please wait for all environments to verify.'));
  }
}
