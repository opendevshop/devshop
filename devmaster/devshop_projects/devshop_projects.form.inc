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
    unset($form['menu']);
    unset($form['revision_information']);
    unset($form['author']);
    unset($form['options']);
    unset($form['buttons']['delete']);
    unset($form['buttons']['preview']);
    if ($form['retry']['#value']) {
      $form['buttons']['submit']['#value'] = t('Save and Retry');
    }
  }
}

/**
 * Implementation of hook_form().
 */
function devshop_projects_form(&$node) {

  $retry = devshop_project_project_create_failed($node->nid, $task);
  if ($retry) {
    $form['notice'] = array(
      '#type' => 'item',
      '#title' => t('NOTICE!'),
      '#description' => t('Project Create failed! You can view the task log ' . l('here.', "node/$task->nid") . ' Please make any necessary corrections below and resubmit the form to try again.'),
      '#weight' => -1,
    );
  }

  $form['git_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Git URL'),
    '#required' => TRUE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $node->git_url,
    '#maxlength' => 255,
    '#weight' => 1,
  );
  $form['title'] = array(
    '#type' => 'textfield',
     '#title' => t('Project Code Name'),
     '#required' => TRUE,
     '#description' => t('Choose a unique name for your project.'),
     '#size' => 40,
     '#default_value' => $node->title,
     '#maxlength' => 255,
  );
  $form['code_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Code path'),
    '#description' => t('The absolute path on the filesystem that will be used to create the platform in the directory specified above.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $node->code_path,
    '#maxlength' => 255,
    '#weight' => 2,
  );
  $form['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary Domain'),
    '#description' => t('The domain name all sites will be built under.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $node->base_url,
    '#maxlength' => 255,
    '#weight' => 2,
  );
  $form['retry'] = array(
    '#type' => 'value',
    '#value' => $retry,
  );

  // Don't allow editing
  if ($node->nid) {
//    $form['code_path']['#type'] = $form['title']['#type'] = 'item';
    $form['code_path']['#value'] = $form['code_path']['#default_value'];
    $form['title']['#value'] = $form['title']['#default_value'];

    $form['code_path']['#attributes'] = array('readonly' => 'readonly');
    $form['title']['#attributes'] = array('readonly' => 'readonly');

    if (!$retry) {
//      $form['git_url']['#type'] = 'item';
      $form['git_url']['#attributes'] = array('readonly' => 'readonly');
      $form['git_url']['#value'] = $form['git_url']['#default_value'];
    }
  }

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function devshop_projects_validate($node, &$form) {

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
  if (!$node->retry && ($result = db_fetch_object(db_query("SELECT name FROM {hosting_context} WHERE name = '%s'", $node->title)))) {
    form_set_error('title', t('Project code name existing in hosting context table.'));
  }

  // The project code name must be unique
  if (!$node->retry && ($result = db_fetch_object(db_query("SELECT title FROM {node} WHERE title = '%s' AND type = 'devshop_project' AND	nid <> %d", $node->title, $node->nid)))) {
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

/**
 *  Form for site installation
 */
function devshop_projects_install_sites_form($form_state, $project_node) {
  
    $form = array();
    $platforms_ready = TRUE;
    $sites_ready = TRUE;
    $sites_installing = FALSE;
    
    // PLATFORMS STATUS: Determine if all platforms are verified.
    if (isset($project_node->project_objects['platform'])){
      foreach ($project_node->project_objects['platform'] as $nid => $env){
        $platform_nodes[$env] = node_load($nid);
        if ($platform_nodes[$env]->platform_status == 0){
          $platforms_ready = FALSE;
        }
      }
    } else {
      $platforms_ready = FALSE;
    }
    // SITES STATUS: Determine if sites exist and are enabled
    if ($platforms_ready && isset($project_node->project_objects['site'])){
      foreach ($project_node->project_objects['site'] as $nid => $env){
        $site_nodes[$env] = node_load($nid);
        if ($site_nodes[$env]->site_status == 0){
          $sites_ready = FALSE;
          $sites_installing = TRUE;
        }
      }
    } else {
      $sites_ready = FALSE;
    }
    
    // OUTPUT
    if ($platforms_ready == FALSE){
      $retry = devshop_project_project_create_failed($project_node->nid, $task);
      if ($retry) {
	$form['note'] = array(
	  '#type' => 'item',
	  '#title' => t('NOTICE!'),
	  '#description' => t('Project Create failed! You can ' . l('view the task log here.', "node/$task->nid") . ' Please ' . l('edit the project settings', "node/$project_node->nid/edit") . ' and make any necessary corrections and resubmit the form to try again.'),
	  '#weight' => -1,
	);
      }
      else {
	$form['note'] = array(
          '#type' => 'item',
	  '#title' => t('Clone & Verify'),
	  '#value' => t('Your code is on its way to the server.  Once the platforms verify, you can choose an installation profile.'),
        );
      }
      return $form;
    } elseif ($sites_installing) {
      $form['note'] = array(
        '#type' => 'item',
        '#title' => t('Sites Installing'),
        '#value' => t('Your Live, Dev, and Test sites are being installed!'),
      );
      return $form;
    } elseif ($sites_ready) {
      return;
    }
    
    // Load all install profiles found on dev platform
    $platform_nid = key($project_node->project_objects['platform']);
    
    $form['ready'] = array(
      '#type' => 'item',
      '#value' => t("Drupal Code has been verified! Now you must create your sites by choosing your installation profile!"),
    );
    
    $profiles = array_combine((array) hosting_get_profiles($platform_nid, 'short_name'), (array) hosting_get_profiles($platform_nid));
    
    // Sensible default?
    // Lets go with standard for now... we can update later.
    if (isset($profiles['standard'])) {
      $default_profile = 'standard';
    }
    // If 'drupal' profile exists, it is likely drupal6!    
    elseif (isset($profiles['drupal'])) {
      $default_profile = 'drupal';
    } 
    $form['install_profile'] = array(
      '#type' => 'radios',
      '#title' => t('Choose your install profile'),
      '#options' => $profiles,
      '#default_value' => $default_profile,
    );
    $form['nid'] = array(
      '#type' => 'value',
      '#value' => $project_node->nid,  
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Create dev, test, and live sites'),  
    );
    return $form;
}

/**
 * Validates for site installation
 */
function devshop_projects_install_sites_form_validate(&$form, &$form_state){
  if (empty($form_state['values']['install_profile'])){
    form_set_error('install_profile', t('You must choose an install profile'));
  }
  
  
}

/**
 * Submit for site installation
 *
 * I believe, based on the fact that provision has to create
 * the site aliases before it runs provision-install, we have
 * to have a new hostmaster task here.
 *
 * If we save the install_profile in the project context,
 * the task could easily create the new sites.
 */
function devshop_projects_install_sites_form_submit(&$form, &$form_state){

  global $user;
  $project_node = node_load($form_state['values']['nid']);

  $project_node->install_profile = $form_state['values']['install_profile'];

  // Save installation profile to database
  db_query('UPDATE {hosting_devshop_project} SET install_profile = "%s" WHERE nid = %d', $form_state['values']['install_profile'], $project_node->nid);
    
  // Create the site nodes
  foreach ($project_node->project_objects['platform'] as $nid => $env){
    devshop_projects_create_site($project_node, node_load($nid), $env);
  }
}

/**
 *  Form for platform creation
 */
function devshop_projects_platform_create_form($form_state, $project_nid) {
  $project_node = node_load($project_nid);

  $form = array();
    
  //Bail if no platforms yet.
  if (!isset($project_node->project_objects['platform'])){
    $retry = devshop_project_project_create_failed($project_nid, $task);
    if ($retry) {
      $form['note'] = array(
	'#type' => 'item',
	'#title' => t('NOTICE!'),
	'#description' => t('Project Create failed! You can ' . l('view the task log here.', "node/$task->nid") . ' Please ' . l('edit the project settings', "node/$project_nid/edit") . ' and make any necessary corrections and resubmit the form to try again.'),
	'#weight' => -1,
      );
    }
    else {
      $form['note'] = array(
        '#type' => 'item',
	'#title' => t('Clone & Verify'),
	'#value' => t('Your code is on its way to the server.  Once the platforms verify, you can choose an installation profile.'),
        );
    }
    return $form;
  }

  $form['platform_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Platform Name'),
    '#required' => TRUE,
    '#description' => t('Enter the name of the platform to be added to project ' . $project_node->title . '. Once the platform is created and verified, a site will be automatically created and added to this project.'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => '',
    '#maxlength' => 255,
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,  
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create platform'),
    '#weight' => 32,
  );
  return $form;
}

/**
 * Validates for platform create
 */
function devshop_projects_platform_create_form_validate(&$form, &$form_state){
  if (empty($form_state['values']['platform_name'])){
    form_set_error('install_profile', t('You must include a platform name'));
  }
  
  
}

/**
 * Submit for platform create
 *
 */
function devshop_projects_platform_create_form_submit(&$form, &$form_state){

  $project_node = node_load($form_state['values']['nid']);
  $args = array();
  if ($form_state['values']['git_url']) {
    $args['git-url'] = $form_state['values']['git_url'];
  }

  if ($form_state['values']['branch']) {
    $args['branch'] = $form_state['values']['branch'];
  }

  $args['platform-name'] = $form_state['values']['platform_name'];

  hosting_add_task($project_node->nid, 'devshop-platform-create', $args);

  $form_state['redirect'] = "node/{$project_node->nid}";
  drupal_goto("node/{$project_node->nid}");
}

/**
 *  Form for project delete
 */
function devshop_projects_project_delete_form($form_state, $project_nid) {
  $project_node = node_load($project_nid);

  $form = devshop_projects_view($project_node)->content;

  unset($form['sites']['add_platform']);
  unset($form['tasks_view']);
  unset($form['devshop']);
  unset($form['info']['#prefix']);
  unset($form['info']['#suffix']);

  $form['remove_dir'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove all project files and directories.'),
    '#description' => t("Note that the platform & site directories are removed regardless of this option. By choosing this option, everything else in the directory '{$project_node->code_path}', including the directory itself, will be removed."),
    '#default_value' => FALSE,
    '#weight' => 30,
  );

  $form['message'] = array(
    '#type' => 'item',
    '#value' => t("Are you sure you want to delete project '{$project_node->title}' and all of its associated sites and platforms?"),
    '#weight' => 31,
  );

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_nid,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete Project'),
    '#weight' => 32,
  );

  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),  
    '#weight' => 33,
  );

  return $form;
}

/**
 *  Form for project delete submit handler
 */
function devshop_projects_project_delete_form_submit(&$form, &$form_state){

  // Make sure the user really wants to delete the project
  if ($form_state['clicked_button']['#value'] == t('Delete Project')) {
    $nid = $form['nid']['#value'];
    $data = devshop_projects_project_data_get($nid);
    
    // First get a list of all of the objects for this project
    $query = db_query("SELECT * " .
		      "FROM {hosting_devshop_project_object} " .
		      "WHERE project_nid = %d", $nid);
    
    $snid = array();
    $pnid = array();
    
    while ($obj = db_fetch_object($query)) {
      if ($obj->object_type == 'site') {
	$snid[] = $obj->object_nid;
      }
      else if ($obj->object_type == 'platform') {
	$pnid[] = $obj->object_nid;
      }
    }
    
    if (count($snid) > 0) {
      $data['project_delete_site_disable'] = $snid;
      $data['project_delete_site_delete'] = $snid;
    }
    else {
      unset($data['project_delete_site_disable']);
      unset($data['project_delete_site_delete']);
    }
    
    if (count($pnid) > 0) {
      $data['project_delete_platform_delete'] = $pnid;
    }
    else {
      unset($data['project_delete_platform_delete']);
    }
    
    if ($data['project_delete_platform_delete'] ||
	$data['project_delete_site_disable']   ||
	$data['project_delete_site_delete'] ) {
      $data['deleting_project'] = TRUE;
      if($form_state['values']['remove_dir']) {
	$data['deleting_project_remove_dir'] = TRUE;
      }
    }
    else {
      unset($data['deleting_project']);
    }

    // Save the delete task and nid lists. We're going to need them
    // when the hosting post task hook gets invoked
    devshop_projects_project_data_set($nid, $data);

    // Kick the whole thing off by deleting the project first
    hosting_add_task($nid, 'delete');
  }

  // Go back to the main project list page
  $form_state['redirect'] = 'hosting/projects';
  drupal_goto('hosting/projects');
}




