<?php
/**
 * @file sync.inc
 * Functions related to the "Sync Data" task.
 */

/**
 * Implementation of hook_hosting_task_TASK_TYPE_form().
 *
 * For "Sync Content" task.
 */
function hosting_task_devshop_sync_form($node) {
  // Get nid
  $nid = $node->nid;
  $project = $node->project;

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the source environment.'), 'source', 'Source');
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the destination environment.  <em>Data from this environment will be DESTROYED.</em>'), 'destination', 'Destination');

  $form['source']['#prefix'] = '<div class="sync-envs"><div class="source">';
  $form['source']['#suffix'] = '</div>';
  $form['destination']['#prefix'] = '<div class="destination">';
  $form['destination']['#suffix'] = '</div></div>';

  // Look for any production mode sites.
  foreach ($project->environments as $environment) {
    if ($environment->settings->production_mode){
      // Remove live site from destination options.
      unset($form['destination']['#options'][$environment->name]);

      $source_options = $form['source']['#options'];
      unset($source_options[$environment->name]);
      $form['source']['#options'] = array_merge(array($environment->name => $environment->name . " <em class='production-mode'>(Production Mode)</em>"), $source_options);
      $form['source']['#default_value'] = $environment->name;
    }
  }

  // Detect pre-selected environments
  if ($_GET['source']) {
    $form['source']['#default_value'] = $_GET['source'];
    $form['destination']['#default_value'] = $_GET['dest'];

    $form['source']['#type'] = 'value';
    $form['destination']['#type'] = 'value';

    $form['source_label'] = array(
      '#type' => 'item',
      '#title' => t('Source Environment'),
      '#value' => $_GET['source'],
      '#description' => t('The environment to copy data from.'),
      '#prefix' => '<div class="sync-envs"><div class="source">',
      '#suffix' => '</div>',
    );
    $form['destination_label'] = array(
      '#type' => 'item',
      '#title' => t('Destination Environment'),
      '#value' => $_GET['dest'],
      '#description' => t("This environment's data will be DESTROYED, replaced with data from the Source."),
      '#prefix' => '<div class="destination">',
      '#suffix' => '</div></div>',
    );
  }

  $form['database'] = array(
    '#title' => t('Copy database from source to destination.'),
    '#type' => 'checkbox',
    '#default_value' => 1,
    '#prefix' => '<fieldset><legend>What to sync</legend>',
  );

  $form['files'] = array(
    '#title' => t('Copy files folder from source to destination.'),
    '#type' => 'checkbox',
    '#default_value' => 0,
    '#suffix' => '</fieldset>',
  );

  // We have to add the fieldset via prefix because of the way aegir handles this form.
  $form['pull'] = array(
    '#title' => t('Pull code on Destination'),
    '#type' => 'checkbox',
    '#default_value' => 1,
    '#prefix' => '<fieldset><legend>Actions</legend>',
  );
  $form['update'] = array(
    '#title' => t('Run update.php on Destination'),
    '#type' => 'checkbox',
    '#default_value' => 1,
  );
  if (_devshop_projects_project_has_module($node, 'features')){
    $form['revert'] = array(
      '#title' => t('Revert all features on Destination'),
      '#type' => 'checkbox',
      '#default_value' => 1,
    );
  }
  $form['cache'] = array(
    '#title' => t('Clear cache on Destination'),
    '#type' => 'checkbox',
    '#default_value' => 1,
    '#suffix' => '</fieldset>',
  );
  $form['#validate'][] = 'hosting_task_devshop_sync_form_validate';
  return $form;
}

/**
 * Validation for hosting_task_devshop_sync_form()
 */
function hosting_task_devshop_sync_form_validate(&$form, &$form_state){
  // Can't sync to self
  if ($form_state['values']['parameters']['source'] == $form_state['values']['parameters']['destination']){
    form_set_error('source', t('The source cannot be the same as the destination.'));
  }
}