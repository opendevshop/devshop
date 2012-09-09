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
  }

  if ($form_id == 'hosting_task_confirm_form'){
    // Get nid
    $nid = $form['nid']['#value'];

    // See if this site is already in a project
    $query = db_query("SELECT project_nid " .
		      "FROM {hosting_devshop_project_object} " .
                      "WHERE object_nid = %d " .
                            "AND object_type = 'site'", $nid);

    if($proj = db_fetch_object($query)) {

      // Only make sites in this project a source for content
      $query = db_query("SELECT object_nid " .
			"FROM {hosting_devshop_project_object} " .
			"WHERE project_nid = %d " .
			"AND object_type = 'site' " .
			"AND object_nid <> %d", $proj->project_nid, $nid);

      $options = array();
      while($s = db_fetch_object($query)) {
	$node = node_load($s->object_nid);
	$options['@' . $node->title] = '@' . $node->title;
      }

      $form['parameters']['source'] = array(
        '#type' => 'radios',
	'#title' => t('Source site alias'),
	'#options' => $options,
	'#description' => t('Select site alias to sync from.'),
	);
    }
  }

}

/**
 * Implementation of hook_form().
 */
function devshop_projects_form(&$node) {

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
  
  // Don't allow editing
  if ($node->nid) {
    $form['code_path']['#type'] = $form['title']['#type'] = $form['git_url']['#type'] = 'item';
    $form['code_path']['#value'] = $form['code_path']['#default_value'];
    $form['title']['#value'] = $form['title']['#default_value'];
    $form['git_url']['#value'] = $form['git_url']['#default_value'];
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
  if ($result = db_fetch_object(db_query("SELECT name FROM {hosting_context} WHERE name = '%s'", $node->title))) {
    form_set_error('title', t('Project code name existing in hosting context table.'));
  }

  // The project code name must be unique
  if ($result = db_fetch_object(db_query("SELECT title FROM {node} WHERE title = '%s' AND type = 'devshop_project' AND	nid <> %d", $node->title, $node->nid))) {
    form_set_error('title', t('Project code name is already is use by another project'));
  }

  // Make sure the path is unique. 
  $cp = hosting_path_normalize($node->code_path);
  if ($add && $result = db_fetch_object(db_query("SELECT code_path FROM {hosting_devshop_project} WHERE code_path = '%s' AND nid <> %d", $cp, $node->nid))) {
    form_set_error('code_path', t('Code path is already in use by another project'));
  }

  // Directory must not exist
  if ($add && file_exists($cp)) {
    form_set_error('code_path', t('Code path directory already exists.'));
  }

  // Sites may not be used in more than one environement
  if($node->dev_site != 0 && $node->dev_site == $node->test_site) {
    form_set_error('dev_site', t('Site used more than once.'));
    form_set_error('test_site', t('Site used more than once.'));
  }

  if($node->dev_site != 0 && $node->dev_site == $node->live_site) {
    form_set_error('dev_site', t('Site used more than once.'));
    form_set_error('live_site', t('Site used more than once.'));
  }

  if($node->test_site != 0 && $node->test_site == $node->live_site) {
    form_set_error('test_site', t('Site used more than once.'));
    form_set_error('live_site', t('Site used more than once.'));
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
      $form['note'] = array(
        '#type' => 'item',
        '#title' => t('Clone & Verify'),
        '#value' => t('Your code is on its way to the server.  Once the platforms verify, you can choose an installation profile.'),
      );
      return $form;
    } elseif ($sites_installing) {
      $form['note'] = array(
        '#type' => 'item',
        '#title' => t('Sites Installing'),
        '#value' => t('Your Live, Dev, and Test sites are being installed!'),
      );
      return $form;
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
    
  // Save installation profile to database
  db_query('UPDATE {hosting_devshop_project} SET install_profile = "%s" WHERE nid = %d', $form_state['values']['install_profile'], $project_node->nid);
    
  // Create the site nodes
  foreach ($project_node->project_objects['platform'] as $nid => $env){
    $node = new stdClass();
    $node->type = 'site';
    $node->status = 1;
    $node->uid = $user->uid;
    $node->title = $env .'.'. $project_node->base_url;

    // Aegir properties
    // @TODO: better client support
    $node->client = HOSTING_DEFAULT_CLIENT;  
    $node->db_server = db_result(db_query('SELECT nid FROM {hosting_db_server}'));
    
    $node->platform = $nid;
    $node->profile = db_result(db_query('SELECT nid FROM {hosting_package} WHERE short_name = "%s"', $form_state['values']['install_profile']));
    
    //$node->port  = db_result(db_query("SELECT ports FROM {hosting_platform} p WHERE p.nid = %d", $nid));
    
    // @TODO: Improve site language handling?
    $node->site_language = !empty($user->language)? $user->language: 'en';
    
    // Save the node
    if ($node = node_submit($node)) {
      node_save($node);
      
      //And save the association to the project
      //INSERT INTO `tesla`.`hosting_devshop_project_object` (`project_nid`, `object_nid`, `object_type`, `env_type`) VALUES ('123', '434', 'ag', 'ag');
      db_query('INSERT INTO {hosting_devshop_project_object} (project_nid, object_nid, object_type, env_type) VALUES (%d, %d, "%s", "%s")', $project_node->nid, $node->nid, $node->type, $env);
    }
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
    $form['note'] = array(
      '#type' => 'item',
      '#title' => t('Clone & Verify'),
      '#value' => t('Your code is on its way to the server.  Once the platforms verify, you can choose an installation profile.'),
      );
    return $form;
  }
    // ZZZ
  $form['platform_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Platform Name'),
    '#required' => TRUE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => '',
    '#maxlength' => 255,
  );
  $form['git_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Git URL'),
    '#required' => FALSE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => '',
    '#maxlength' => 255,
  );
  $form['branch'] = array(
    '#type' => 'textfield',
    '#title' => t('Branch'),
    '#required' => TRUE,
    '#description' => t(''),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => 'master',
    '#maxlength' => 255,
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,  
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create platform'),
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
 * Submit for site installation
 *
 * I believe, based on the fact that provision has to create
 * the site aliases before it runs provision-install, we have
 * to have a new hostmaster task here.
 *
 * If we save the install_profile in the project context,
 * the task could easily create the new sites.
 */
function devshop_projects_platform_create_form_submit(&$form, &$form_state){

  global $user;
  $project_node = node_load($form_state['values']['nid']);
    
  // Save installation profile to database
  db_query('UPDATE {hosting_devshop_project} SET install_profile = "%s" WHERE nid = %d', $form_state['values']['install_profile'], $project_node->nid);
    
  // Create the site nodes
  foreach ($project_node->project_objects['platform'] as $nid => $env){
    $node = new stdClass();
    $node->type = 'site';
    $node->status = 1;
    $node->uid = $user->uid;
    $node->title = $env .'.'. $project_node->base_url;

    // Aegir properties
    // @TODO: better client support
    $node->client = HOSTING_DEFAULT_CLIENT;  
    $node->db_server = db_result(db_query('SELECT nid FROM {hosting_db_server}'));
    
    $node->platform = $nid;
    $node->profile = db_result(db_query('SELECT nid FROM {hosting_package} WHERE short_name = "%s"', $form_state['values']['install_profile']));
    
    //$node->port  = db_result(db_query("SELECT ports FROM {hosting_platform} p WHERE p.nid = %d", $nid));
    
    // @TODO: Improve site language handling?
    $node->site_language = !empty($user->language)? $user->language: 'en';
    
    // Save the node
    if ($node = node_submit($node)) {
      node_save($node);
      
      //And save the association to the project
      //INSERT INTO `tesla`.`hosting_devshop_project_object` (`project_nid`, `object_nid`, `object_type`, `env_type`) VALUES ('123', '434', 'ag', 'ag');
      db_query('INSERT INTO {hosting_devshop_project_object} (project_nid, object_nid, object_type, env_type) VALUES (%d, %d, "%s", "%s")', $project_node->nid, $node->nid, $node->type, $env);
    }
  }
}
