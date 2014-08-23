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
  
  // Hosting Task Forms
  if ($form_id == 'hosting_task_confirm_form') {
    switch($form['task']['#value']) {

      // Create Environment form.
      case 'devshop-create':
        drupal_set_title(t('Create new environment'));
        $form['#submit'] = array('hosting_task_devshop_create_form_submit');
        break;

      // Migrate Form: used for changing database server.
      case 'migrate':

        // To change the database server, we use the migrate form.
        if ($_GET['db_server']) {
          drupal_set_title(t('Migrate Database'));
          $site_node = node_load($form['nid']['#value']);
          $environment = $site_node->environment;
          $form['help']['#value'] = t("Are you sure you want to change this site's database server?");

          // Set DB Server values
          $form['parameters']['new_db_server']['#type'] = 'value';
          $db_server = $_GET['db_server'];
          $db_server_nid = db_result(db_query('SELECT nid FROM {node} WHERE type = "server" && title = "%s"', $db_server));
          $form['parameters']['new_db_server']['#default_value'] = $db_server_nid;

          // Set URI value
          $form['parameters']['new_uri']['#type'] = 'value';
          $form['parameters']['new_uri']['#value'] = $form['parameters']['new_uri']['#default_value'];
          // Display something helpful
          $form['old'] = array(
            '#type' => 'item',
            '#title' => t('Current Database Server'),
            '#value' => l($environment->servers['db']['name'], 'node/' . $environment->servers['db']['nid']),
            '#weight' => '-1',
          );

          // Display something helpful
          $form['new'] = array(
            '#type' => 'item',
            '#title' => t('New Database Server'),
            '#value' => l($db_server, "node/$db_server_nid"),
            '#weight' => '0',
          );

          // @TODO: not sure how to hide the radios.  We'll just use CSS for now.
          $form['parameters']['#prefix'] = '<div class="hidden">';
          $form['parameters']['#suffix'] = '</div>';

        }
        break;
    }
  }

  // Platform "Edit" page.
  if ($form_id == 'platform_node_form') {
   dsm($form);

    $platform_node = node_load($form['nid']['#value']);
    $environment = $platform_node->environment;
    $form['help'] = array(
      '#value' => t("Are you sure you want to change this site's web server?  <strong>NOTE: You will have to change DNS as well!</strong>"),
      '#weight' => -10,
    );

    // Set to values
    $form['title']['#type'] = 'value';
    $form['makefile']['#type'] = 'value';
    unset($form['info']);

    if ($_GET['web_server']){
      $web_server = $_GET['web_server'];
      $web_server_nid = db_result(db_query('SELECT nid FROM {node} WHERE type = "server" && title = "%s"', $web_server));

      // Set values
      $form['web_server']['#default_value'] = $web_server_nid;
      $form['web_server']['#type'] = 'hidden';

      // Load servers to display IP addresses.
      $old_server = node_load($environment->servers['http']['nid']);
      $new_server = node_load($web_server_nid);

      // Display something helpful
      $form['old'] = array(
        '#type' => 'item',
        '#title' => t('Current Web Server'),
        '#value' => l($environment->servers['http']['name'], 'node/' . $environment->servers['http']['nid']) . '<br />' . implode($old_server->ip_addresses, '<br />'),
        '#weight' => '-1',
      );

      // Display something helpful
      $form['new'] = array(
        '#type' => 'item',
        '#title' => t('New Web Server'),
        '#value' => l($web_server, "node/$web_server_nid") . '<br />' . implode($new_server->ip_addresses, '<br />'),
        '#weight' => '0',
      );


    }


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

    // Skip this if it is not enabled.
    if ($environment->site_status == HOSTING_SITE_DELETED && $environment->platform_status == HOSTING_PLATFORM_DELETED) {
      continue;
    }

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
        '#type' => 'value',
        '#value' => $environment->git_ref,
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
    '#title' => t('Domain Name Settings'),
  );

  // Live Domain
  $form['project']['settings']['live']['live_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('Live domain'),
    '#description' => t('The live domain for this project. Do not include "www".'),
    '#size' => 40,
    '#default_value' => $node->project->settings->live['live_domain'],
  );

  // Live Environment
  $environments = array_keys($node->project->environments);
  $environments_options = array_combine($environments, $environments);
  $environments_options[''] = t('No live environment');

  $form['project']['settings']['live']['live_environment'] = array(
    '#type' => 'select',
    '#title' => t('Live environment'),
    '#description' => t('The environment for the live website.'),
    '#options' => $environments_options,
    '#default_value' => $node->project->settings->live['live_environment'],
  );

  // Use www
  $form['project']['settings']['live']['live_domain_www'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add www as a ServerAlias for Live Environment'),
    '#description' => t('Access the site at http://livedomain.com and http://www.livedomain.com'),
    '#default_value' => $node->project->settings->live['live_domain_www'],
  );

  // Use aliases
  $form['project']['settings']['live']['environment_aliases'] = array(
    '#type' => 'checkbox',
    '#title' => t('Environment Subdomains'),
    '#description' => t('Create subdomains under the live domain for all environments.'),
    '#default_value' => $node->project->settings->live['environment_aliases'],
  );

  return $form;
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
}
