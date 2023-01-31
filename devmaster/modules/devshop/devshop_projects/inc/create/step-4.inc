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
  );
  $all_tasks_queued = TRUE;
  $all_tasks_succeeded = TRUE;
  $modals = '';
  foreach ($project->environments as $name => $environment) {

    // Get platform and latest verify task.
    $platform_nid = $environment->platform;
    $platform = node_load($platform_nid);
    $task = hosting_get_most_recent_task($platform_nid, 'verify');
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
    $status_link = devshop_get_logs_and_permalink($task);

    // Build a table.
    $row = array();
    $row['name'] = $name;
    $row['branch'] = $environment->git_ref;
//    $row['version'] = t('...');
//    $row['profiles'] = '';
    $row['status'] = $status_link;

    // If platform verified successfully:
    if ($task->task_status == HOSTING_TASK_SUCCESS || $task->task_status == HOSTING_TASK_WARNING) {

      // It's not really ready until we get a version.
      if (empty($platform->release->version)) {
        $completed = FALSE;
        $row['version'] = '...';
        $row['profiles'] = '';
      }
      else {
        $row['version'] = "<label id='version-{$name}'>" . $platform->release->version;
        $row['profiles'] = '';
      }

      // Collect install profiles
//      $profiles_shortnames = hosting_get_profiles($platform->nid, 'short_name');
//
//      if (is_array($profiles_shortnames) && !empty($profiles_shortnames)) {
//        $profiles[$name] = array_combine($profiles_shortnames, (array) hosting_get_profiles($platform->nid));
//        $row['profiles'] = implode(', ', $profiles[$name]);
//      }
//      else {
//        $profiles[$name] = array();
//      }

//      // If no tasks have failed, save available profiles
//      if ($all_tasks_succeeded) {
//        if (empty($available_profiles)) {
//          $available_profiles = $profiles[$name];
//        }
//        else {
//          $available_profiles = array_intersect_key($available_profiles, $profiles[$name]);
//        }
//      }
    }
    // If platform verification failed:
    elseif ($task->task_status == HOSTING_TASK_ERROR) {
      $completed = TRUE;
      $all_tasks_succeeded = FALSE;
//      $available_profiles = array();

      $messages = devshop_task_get_messages($task);
      $errors_rendered = implode("\n", $messages);
      $codebase_verification_failed = t('Codebase Verification Failed');
      $summary = t('in %path', array(
        '%path' => $environment->root,
      ));
      $close = t('Close');
      $modals .= <<<HTML

      <!-- Modal -->
      <div class="modal fade" id="errors-modal-{$name}" tabindex="-1" role="dialog" aria-labelledby="errors-modal" aria-hidden="true">
        <div class="modal-dialog">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">&times;</span><span class="sr-only">Close</span></button>
              <h4 class="modal-title" id="errors-modal">$codebase_verification_failed <small>$summary</small></h4>
            </div>
            <div class="modal-body">
            $errors_rendered
            </div>
            <div class="modal-footer">
              <button type="button" class="btn btn-default" data-dismiss="modal">$close</button>
            </div>
          </div>
        </div>
      </div>
    </div>
HTML;
      $row['version'] = "<label id='version-$name'>" . (!empty($platform->release->version)? $platform->release->version: $task->status_name) . "</label>";
    }
    // If platform is still processing:
    elseif ($task->task_status == HOSTING_TASK_PROCESSING || $task->task_status == HOSTING_TASK_QUEUED) {
      $completed = FALSE;
      $row['version'] = "<label id='version-{$name}'>{$task->status_name}</label>";
      $row['profiles'] = "<span id='profiles-{$name}'></span>";
    }

    // If a single task is not queued, $all_tasks_queued == FALSE
    if ($task->task_status != HOSTING_TASK_QUEUED) {
      $all_tasks_queued = FALSE;
    }

    // If a single task is not queued, $all_tasks_queued == FALSE
    if ($task->task_status == HOSTING_TASK_ERROR) {
      $task_failed = TRUE;
    }

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
    $note = '<div id="message" class="alert alert-info">' . t('Please wait while we clone your repo and install your Drupal code. This may take a while depending on the size of your repo and the number of Composer dependencies.') . '</div>';
    $note .= '<div id="progress-indicator"><p class="wait"><i class="fa fa-spinner fa-spin fa-4x text-primary"></i></p></div>';
    $note .= '<div id="button"></div>';

    $form['help'] = array(
      '#markup' => $note,
    );

    // Add code to reload the page when complete.
    devshop_form_reloader($form, 'platform');
  }
  elseif ($task_failed) {
    $project->no_finish = TRUE;

    $message = t('Something went wrong when trying to clone your codebases.');

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
      '#prefix' => '<div id="message" class="alert alert-danger">',
      '#markup' => $message,
      '#suffix' => '</div>',
    );
    $form['retry'] = array(
      '#type' => 'submit',
      '#action' => 'retry',
      '#value' => t('Pull code and try again'),
      '#prefix' => '<div id="button" class="text-center">',
      '#suffix' => '</div>',
    );
    $form['#no_finish'] = TRUE;
  }

  // If ready...
  else {

//    // If no common profiles found, just set to standard.
//    if (count($available_profiles) == 0) {
//      $available_profiles['standard'] = 'No default profile.';
//      $default_profile = 'standard';
//    }

    $project->no_finish = FALSE;

//    // Install Profile
//    // Sensible default?
//    // Lets go with standard for now... we can update later.
//    if (isset($available_profiles['standard'])) {
//      $default_profile = 'standard';
//    }
//    // If 'drupal' profile exists, it is likely drupal6!
//    elseif (isset($available_profiles['drupal'])) {
//      $default_profile = 'drupal';
//    }
    $default_profile = 'default';
    $form['install_profile'] = array(
      '#type' => 'textfield',
      '#title' => t('Install Profile'),
      '#description' => t('The installation profile you would like to install sites with (optional).'),
      '#default_value' => '',
    );
  }

  if (variable_get('hosting_queued_paused')) {
    $form['messages'] = array(
      '#type' => 'container',
      '#children' => t('The task queue is paused. Visit !link to unpause.', array('!link' => l(t('Hosting Queue Daemon Settings'), 'admin/hosting/queued', array('query' => drupal_get_destination())))),
      '#attributes' => array(
        'class' => array('alert alert-warning'),
      ),
    );
  }

  $form['#suffix'] = $modals;
  return $form;
}

/**
 * STEP 4: Validate
 */
function devshop_project_create_step_sites_validate(&$from, &$form_state) {
  if ($form_state['triggering_element']['#action'] == 'retry') {
    foreach ($form_state['project']->environments as $name => $environment) {
      hosting_add_task($environment->platform, 'verify');
    }
    // Reload project and environnt data so latest tasks are present.
    $form_state['redirect'] = 'projects/add/sites';
  }
}
