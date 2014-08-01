<?php
/**
 * @file devshop_project.form.inc DevShop Project forms related hooks and
 * support function include file.
 */

/**
 * Implements hook_form_alter().
 */
function devshop_projects_form_alter(&$form, &$form_state, $form_id){

  // Removing unneccesary fieldgroups
  if ($form_id == 'project_node_form'){
    global $user;

    $project = $form['#node'];

    unset($form['menu']);
    unset($form['revision_information']);
    unset($form['author']);
    unset($form['options']);
    unset($form['buttons']['delete']);
    unset($form['buttons']['preview']);
    if ($form['retry']['#value']) {
      $form['buttons']['submit']['#value'] = t('Save and Retry');
    }

    //Add button for delete project
    $form['buttons']['delete'] = array(
      '#type' => 'markup',
      '#value' => l(t('Delete this project'), 'node/'. $project->nid .'/project_delete', array('query' => array('token' => drupal_get_token($user->uid)))),
      '#weight' => 10,
    );
  }

  // Create Project Wizard
  if ($form_id == 'devshop_project_create_step_sites' || $form_id == 'devshop_project_create_step_settings' || $form_id == 'devshop_project_create_step_environments'){
    if ($form_state['project']->no_finish){
      unset($form['buttons']['return']);
    }
     if ($form_state['project']->no_next){
      unset($form['buttons']['next']);
    }
  }
  
  // On Hosting Task: Create Project form, do our own submission.
  if ($form_id == 'hosting_task_confirm_form' && $form['task']['#value'] == 'devshop-create'){
    drupal_set_title(t('Create new environment'));
    $form['#submit'] = array('hosting_task_devshop_create_form_submit');
  }

   // On Hosting Task: Delete Project form, do our own submission.
  if ($form_id == 'hosting_task_confirm_form' && $form['task']['#value'] == 'devshop-delete') {
    $node = node_load($form['nid']['#value']);

    if ($node->type == 'project') {
      $form['#submit'] = array('hosting_task_devshop_delete_form_submit');
    }
  }


  // If not a part of a project, bail out.
  if (empty($form['#node']->nid)){
    return;
  }

  // Save values that need to be saved.
  if ($form_id == 'platform_node_form' || $form_id == 'site_node_form'){
    $node = $form['#node'];
    $form['project'] = array(
      '#type' => 'value',
      '#value' => $node->project,
    );
    $form['environment'] = array(
      '#type' => 'value',
      '#value' => $node->environment,
    );
    $form['git_branch'] = array(
      '#type' => 'value',
      '#value' => $node->git_branch,
    );
  }
}


/**
 * Implementation of hook_form().
 */
function devshop_projects_form(&$node) {

  $project = $node->project;

  // Project Settings
  // Every value under $form['project'] gets serialized and saved into a project's "data" column.
  // @TODO: Add ['data'] so only items under 'data' get serialized.
  $form['project'] = array(
    '#tree' => TRUE,
  );

  // Hidden fields that can't change.
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Project Code Name'),
    '#required' => TRUE,
    '#description' => t('Choose a unique name for your project.'),
    '#size' => 40,
    '#default_value' => $node->title,
    '#maxlength' => 255,
  );
  $form['project']['git_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Git URL'),
    '#required' => TRUE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->git_url,
    '#maxlength' => 255,
    '#weight' => 1,
  );
  $form['project']['code_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Code path'),
    '#description' => t('The absolute path on the filesystem that will be used to create the platform in the directory specified above.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->code_path,
    '#maxlength' => 255,
    '#weight' => 2,
  );
  $form['project']['drupal_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Drupal'),
    '#description' => t('Enter the relative path to the index.php file in your repository. Leave blank if index.php is in the root.'),
    '#size' => 40,
    '#default_value' => $project->drupal_path,
    '#maxlength' => 255,
    '#weight' => 3,
  );
  $form['project']['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Domain'),
    '#description' => t('The domain name all sites will be built under.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->base_url,
    '#maxlength' => 255,
    '#weight' => 4,
  );

  // Don't allow editing
  if ($node->nid) {

    // Title
    $form['title']['#value'] = $form['title']['#default_value'];
    $form['title']['#type'] = 'value';

    // Other fields
    $locked = array('git_url', 'code_path', 'drupal_path', 'base_url');
    foreach ($locked as $field){
      $form['project'][$field]['#value'] = $form['project'][$field]['#default_value'];
      $form['project'][$field]['#type'] = 'value';
    }
  }

  // Environment settings
  $form['project']['environments'] = array(
    '#type' => 'fieldset',
    '#title' => t('Environment Settings'),
    '#theme' => 'devshop_projects_settings_form',
    '#tree' => TRUE,
  );

  $live_alias_options = array('<none>');
  $settings = module_invoke_all('devshop_project_settings', $node);
  foreach ($node->project->environments as $environment_name => $environment) {
      $form['project']['environments'][$environment_name] = array(
        '#tree' => TRUE,
        '#title' => $environment_name,
        '#theme' => 'devshop_projects_settings_table',
      );

      // Save current environment settings to detect differences later.
      $form['project']['environments'][$environment_name]['orig'] = array(
        '#type' => 'value',
        '#value' => $environment,
      );

      // Environment properties
      $form['project']['environments'][$environment_name]['site'] = array(
        '#type' => 'value',
        '#value' => $environment->site,
      );
      $form['project']['environments'][$environment_name]['platform'] = array(
        '#type' => 'value',
        '#value' => $environment->platform,
      );
      $form['project']['environments'][$environment_name]['git_ref'] = array(
        '#title' => t('Git Branch/Tag'),
        '#type' => 'select',
        '#options' => devshop_projects_git_ref_options($project),
        '#default_value' => $environment->git_ref,
      );

      $form['project']['environments'][$environment_name]['settings'] = array();

      // Environment settings
      foreach ($settings as $setting_id => $setting){
        $form['project']['environments'][$environment_name]['settings'][$setting_id] = $setting;
        $form['project']['environments'][$environment_name]['settings'][$setting_id]['#default_value'] = $environment->settings->{$setting_id};
        $form['project']['environments'][$environment_name]['settings'][$setting_id]['#attributes']['title'] = $setting['#description'];
      }

      $site = node_load($environment->site);
      $live_alias_options[$environment_name] = $site->hosting_name;
  }

  // Project Settings
  // Save git branches and tags
  $form['project']['settings']['git']['branches'] = array(
    '#type' => 'value',
    '#value' => $node->project->data->git['branches'],
  );
  $form['project']['settings']['git']['tags'] = array(
    '#type' => 'value',
    '#value' => $node->project->data->git['tags'],
  );

  // Live Environment settings.
  $form['project']['settings']['live'] = array(
    '#type' => 'fieldset',
    '#title' => t('Live Environment Settings'),
  );

//  @TODO: Not ready yet!  Rethink this.
//    1. We should probably allow a "production mode" on environments, disabling sync data destination and allowing for other things.
//    2. We should break out domain management into it's own thing.  The way this is setup is hard to understand.
//


//  $live_alias_options['other'] = 'Other...';
//  if (empty($live_alias_options[$node->project->settings->live['live_environment']])){
//    // No live environment
//    $live_environment_default_value = '';
//  }
//  elseif (!empty($node->project->settings->live['live_environment_alias'])){
//    // "other" live environment
//    $live_environment_default_value = 'other';
//  }
//  else {
//    $live_environment_default_value = $node->project->settings->live['live_environment'];
//  }
//
//  $form['project']['settings']['live']['live_environment'] = array(
//    '#type' => 'select',
//    '#title' => t('Live Environment'),
//    '#options' => $live_alias_options,
//    '#required' => FALSE,
//    '#default_value' => $live_environment_default_value,
//    '#description' => t('Choose which environment is your live production environment. This will prevent users from Syncing data over the live site, and allow easy "Sync From Live" functionality.'),
//  );
//  $form['project']['settings']['live']['live_environment_alias'] = array(
//    '#title' => t('Live Site Drush Alias'),
//    '#type' => 'textfield',
//    '#description' => t("If your 'live' site is outside of this server, add a remote alias file to /var/aegir/.drush and enter the alias name here."),
//    '#field_prefix' => '@',
//    '#size' => 40,
//    '#default_value' => $node->project->live['live_environment_alias'],
//    '#size' => 50,
//    '#maxlength' => 50,
//  );

  // Live Domain
  $form['project']['settings']['live']['live_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('Live domain'),
    '#description' => t('The live domain for this project. Do not include "www".'),
    '#size' => 40,
    '#default_value' => $node->project->live['live_domain'],
    '#weight' => 5,
  );

  // Use aliases
  if (module_exists('hosting_alias')){
    $form['project']['settings']['live']['live_domain_aliases'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use environment aliases'),
      '#description' => t('Create aliases for all environments as subdomains of the live domain.'),
      '#default_value' => $node->project->live['live_domain_aliases'],
      '#weight' => 6,
    );
  }

  return $form;
}

/**
 * Submit function for save data of platforms.
 */
function devshop_projects_submit_settings($form, &$form_state) {
//
//  // Go through and save our settings to site and platform nodes.
//  $project_node = node_load($form_state['values']['nid']);
//  $settings_info = module_invoke_all('devshop_project_settings', $project_node);
//  $nodes = array();
//
//  // Save all items under project to data
//  // $form_state['values']['project']['data'] = (object) $form_state['values']['project'];
//
//  // Go through each environment
//  // @TODO: Move to devshop_project_update();
//  foreach ($form_state['values']['project']['environments'] as $environment_name => $settings){
//    //$form_state['values']['project']['environments'][$environment_name]['settings'] = $form_state['values']['project']['environments'][$environment_name];
//
//    // Then through each setting of each environment
//    foreach ($settings as $setting_name => $setting_value){
//
//      //Find the node type for this setting.
//      $node_type = $settings_info[$setting_name]['#node_type'];
//
//      // Load the site or platform node for this environment, then set the value and save.
//      if (!empty($project_node->project->environments[$environment_name]->{$node_type})){
//        $nid = $project_node->project->environments[$environment_name]->{$node_type};
//        $nodes[$nid] = node_load($nid);
//        $nodes[$nid]->no_verify = TRUE;
//
//        //If changed database then execute migrate task.
//        if ($setting_name == 'db_server' && $nodes[$nid]->{$setting_name} != $setting_value) {
//          $args['target_platform'] = $nodes[$nid]->platform;
//          $args['new_uri'] = $nodes[$nid]->title;
//          $args['new_db_server'] = $setting_value;
//
//          hosting_add_task($nid, 'migrate', $args);
//        }
//        $nodes[$nid]->{$setting_name} = $setting_value;
//      }
//    }
//  }
//
//  // Go save all nodes
//  foreach ($nodes as $nid => $node){
//    node_save($node);
//  }
//
}

/**
 * Implementation of hook_validate().
 */
function devshop_projects_validate($node, &$form) {

  // It may look find of strange that we are doing a node load when the
  // exact node we want is passed in here. But if this is an edit of an
  // existing node, not all of the node fields (like project_status) have
  // been initialized yet.

  $node = node_load($node->nid);

  // No validation if op == Delete
  if ($node->op == t('Delete')) {
    return;
  }

  // Full validation on when a creating a new node
  $add = (arg(1) == 'add' ? TRUE : FALSE);

  // Title (project code) must not have any spaces
  if(strpos($node->title, ' ') != FALSE) {
    form_set_error('title', t('Project code name must not contain any white spaces.'));
  }
  // The project code name must not be in the hosting_context table
  if (!$node->retry){
    $result = db_fetch_object(db_query("SELECT name, nid FROM {hosting_context} WHERE name = '%s'", $node->title));
    if ($node->nid != $result->nid){

      form_set_error('title', t('Project code name is unavailable.  Please choose another.'));
    }
  }

  // The project code name must be unique
  if (!$node->retry && ($result = db_fetch_object(db_query("SELECT title FROM {node} WHERE title = '%s' AND type = 'devshop_project' AND nid <> %d", $node->title, $node->nid)))) {
    form_set_error('title', t('Project code name is already is use by another project'));
  }

  // Make sure the path is unique.
  $cp = hosting_path_normalize($node->code_path);
  if (!$node->retry && $add && $result = db_fetch_object(db_query("SELECT code_path FROM {hosting_devshop_project} WHERE code_path = '%s' AND nid <> %d", $cp, $node->nid))) {
    form_set_error('code_path', t('Code path is already in use by another project'));
  }

  // Directory must not exist
  if (!$node->retry && $add && file_exists($cp)) {
    form_set_error('code_path', t('Code path directory already exists.'));
  }

  // If live environment is selected, do not save live environment alias
  if ($node->live_environment == 'other'){
    $node->live_environment = $node->live_environment_alias;
  }
  else {
    $node->live_environment_alias = '';
  }
}
