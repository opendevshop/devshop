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

  $form = array();
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the source environment.'), 'source', 'Source');
  devshop_projects_tasks_add_environment_to_form($form, $node, t('Choose the destination environment.  <em>Data from this environment will be DESTROYED.</em>'), 'destination', 'Destination');

  $form['source']['#prefix'] = '<div class="sync-envs"><div class="source">';
  $form['source']['#suffix'] = '</div>';
  $form['destination']['#prefix'] = '<div class="destination">';
  $form['destination']['#suffix'] = '</div></div>';

  if (!empty($form['destination']['#options'][$node->live_environment])){
    // Remove live site from destination options.
    unset($form['destination']['#options'][$node->live_environment]);

    $source_options = $form['source']['#options'];
    unset($source_options[$node->live_environment]);
    $form['source']['#options'] = array_merge(array($node->live_environment => $node->live_environment . " <em>(LIVE ENVIRONMENT)</em>"), $source_options);
    $form['source']['#default_value'] = $node->live_environment;
  }
  elseif (!empty($node->live_environment_alias)){
    $form['source']['#options']["@" . $node->live_environment_alias] = "@" . $node->live_environment_alias . " <em>(LIVE ENVIRONMENT)</em>";
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
      '#default_value' => $has_features,
      '#access' => $has_features,
    );
  } else{
    // I think its better UI just to hide it? If they need features it will be enabled!
    //$form['actions']['revert'] = array(
    //  '#title' => t('<em>Revert all features on Destination after content sync?</em>'),
    //  '#type' => 'checkbox',
    //  '#description' => t('This site doesn\'t have features.module enabled. Please enable and then "Verify" the site.'),
    //  '#default_value' => FALSE,
    //  '#disabled' => TRUE,
    //  );
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