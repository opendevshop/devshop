<?php

/**
 * Menu Access callback for Environment Create Form
 */
function devshop_projects_environment_add_access($project_name) {
  $node = devshop_projects_load_by_name($project_name);
  return $node->type == 'project' && $node->status == NODE_PUBLISHED && node_access('create', 'site');
}
/**
 * Menu page callback for Environment Create Page.
 */
function devshop_projects_environment_add_page($project_name) {
  global $user;
  $node = devshop_projects_load_by_name($project_name);

  $page['help'] = array(
    '#markup' => t('There are many ways to create a new copy of your website. You can install Drupal from a profile, clone an existing environment, or import a database dump from another location.'),
    '#prefix' => '<div class="alert alert-info"><i class="fa fa-info-circle"></i> ',
    '#suffix' => '</div>',
  );
  $page['form'] = drupal_get_form('devshop_environment_add_form', $node);
  return $page;
}

/**
 * Form function for Add Environment.
 *
 * Includes the site_node_form
 */
function devshop_environment_add_form($form, $form_state, $project_node) {
  global $user;
  $project = $project_node->project;
  $branch_options = devshop_projects_git_ref_options($project);

  // We are creating our own form, but we want to let other modules alter it as
  // if it were the site_node_form. This is how drupal_prepare_form() invokes
  // hook_form_alter() and hook_form_FORM_ID_alter() hooks.
  $form = array(
    '#prefix' => '<div class="container add-environment-container">',
    '#suffix' => '</div>',
  );
  $hooks = array('form');
  $hooks[] = 'form_node_form';
  $hooks[] = 'form_site_node_form';
  $form_id = 'site_node_form';
  drupal_alter($hooks, $form, $form_state, $form_id);

  // Remove unwanted items
  unset($form['menu']);
  unset($form['git']);

  // Generate field prefix and suffix using domain name pattern.
  // @TODO: When per-project patterns are added, look for that setting instead of the variable.
  $pattern = variable_get('devshop_project_environment_url_pattern', '@environment.@project.@hostname');

  $labels = explode('@environment', strtr($pattern, array(
    '@project' => $project_node->title,
    '@hostname' => $_SERVER['SERVER_NAME'],
  )));

  $form['environment_name'] = array(
    '#title' => t('Environment Name'),
    '#type' => 'textfield',
    '#description' => t('Enter a system name for your environment.  For consistency, you might want to match the branch name.'),
    '#required' => TRUE,
    '#field_prefix' => '<div class="input-group"><div class="input-group-addon">http://' . $labels[0] . '</div>',
    '#field_suffix' => '<div class="input-group-addon input-group-addon-url">' . $labels[1] .'</div>
        </div>',
    '#size' => 10,
    '#maxlength' => 64,
    '#weight' => -100,
    '#attributes' => array(
      'data-placement' => 'bottom',
    ),
    '#wrapper_attributes' => array(
      'class' => array('col-sm-8 col-md-8'),
    ),
  );

  $form['git_ref'] = array(
    '#title' => t('Branch or Tag'),
    '#type' => 'select',
    '#options' => $branch_options,
    '#required' => TRUE,
    '#weight' => -99,
    '#wrapper_attributes' => array(
      'class' => array('col-sm-4 col-md-4'),
    ),
  );

  $form['install_method'] = array(
    '#type' => 'fieldset',
    '#weight' => -99,
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('clearfix'),
    ),
  );
  $form['install_method']['method'] = array(
    '#type' => 'radios',
    '#required' => TRUE,
    '#title' => t('Install Method'),
    '#process' => array('devshop_environment_method_process'),
    '#weight' => -99,
    '#options' => array(
      'manual' => t('Manual Install'),
      'profile' => t('Drupal Profile'),
      'clone' => t('Clone Environment'),
      'import' => t('Import Database'),
    ),
  );

  $form['install_method']['none'] = array(
    '#type' => 'container',
    '#weight' => -98,
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[method]"]' => array('value' => 'manual'),
      ),
    ),
    'note' => array(
      '#markup' => t('An empty database will be created. You can install Drupal manually by visiting install.php, by using Drush, or you can manually import a database.'),
    ),
  );

  $form['install_method']['profile'] = array(
    '#type' => 'radios',
    '#title' => t('Drupal Install Profile'),
    '#weight' => -98,
    '#required' => TRUE,
    '#options' => devshop_environment_add_form_profile_options($project),
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[method]"]' => array('value' => 'profile'),
      ),
    ),
  );

  // Set default profile.
  if (count($form['install_method']['profile']['#options']) == 1) {
    $form['install_method']['profile']['#default_value'] = '_other';
  }
  else {
    $form['install_method']['profile']['#default_value'] = $project->settings->default_environment['install_profile'];
  }

  $form['install_method']['profile_other'] = array(
    '#type' => 'textfield',
    '#title' => t('Other Install Profile'),
    '#title_display' => 'invisible',
    '#description' => t('This Drupal profile must exist in your codebase for installation to succeed.'),
    '#attributes' => array(
      'placeholder' => t('Profile Name'),
    ),
    '#weight' => -97,
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[method]"]' => array('value' => 'profile'),
        ':input[name="install_method[profile]"]' => array('value' => '_other'),
      ),
    ),
  );

  // Generate environments options.
  foreach ($project->environments as $e => $environment) {
    $environment_options[$e] = $environment->name . ' ' . l($environment->url, $environment->url, array(
      'attributes' => array('target' => '_blank'),
    ));
  }
  $environment_options['drush'] = t('Other Drush Alias');

  if (count($environment_options)) {
    $form['install_method']['clone'] = array(
      '#type' => 'radios',
      '#title' => t('Environment to clone'),
      '#weight' => -98,
      '#required' => TRUE,
      '#options' => $environment_options,
      '#default_value' => key($environment_options),
      '#states' => array(
        'visible' => array(
          ':input[name="install_method[method]"]' => array('value' => 'clone'),
        ),
      ),
    );
  }

  $form['install_method']['clone_source_drush'] = array(
    '#type' => 'textfield',
    '#title' => t('Drush Alias'),
    '#description' => t('Enter a Drush alias you would like to Sync your database and files from. It must already exist on the server.'),
    '#weight' => -98,
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[clone]"]' => array('value' => 'drush'),
        ':input[name="install_method[method]"]' => array('value' => 'clone'),
      ),
    ),
  );

  $form['install_method']['import'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to SQL'),
    '#description' => t('Enter either a remote MySQL address (such as mysql://username:password@host/database), or an absolute path to an SQL dump (such as /var/aegir/site-backup.sql).'),
    '#weight' => -98,
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[method]"]' => array('value' => 'import'),
      ),
    ),
  );

  // Server Selection
  $form['server_stack'] = array(
    '#type' => 'fieldset',
    '#title' => t('Server Stack'),
    '#group' => 'environment_settings',
  );

  // Choose database server.
  $db_servers = hosting_get_servers('db');
  if (sizeof($db_servers) > 1) {
    $form['server_stack']['db_server'] = array(
      '#type' => 'radios',
      '#title' => t('Database server'),
      '#required' => true,
      '#description' => t(
        'The database server the site will use to host its content.'
      ),
      '#options' => $db_servers,
      '#default_value' => $project->settings->default_environment['db_server'],
    );
  }
  elseif (sizeof($db_servers) == 1) {
    $server_name = current($db_servers);
    $server_nid = key($db_servers);
    $form['server_stack']['db_server_label'] = array(
      '#type' => 'item',
      '#title' => t('Database server'),
      '#markup' => l($server_name, "node/$server_nid", array('attributes' => array('target' => '_blank'))),
    );
  }

  $web_servers = hosting_get_servers('http');
  if (sizeof($web_servers) > 1) {
    $form['server_stack']['web_server'] = array(
      '#type' => 'radios',
      '#title' => t('Web server'),
      '#description' => t('The web server the sites will be hosted on.'),
      '#options' => $web_servers,
      '#default_value' => $project->settings->default_environment['web_server'],
    );
  }
  elseif (sizeof($web_servers) == 1) {
    $server_name = current($web_servers);
    $server_nid = key($web_servers);
    $form['server_stack']['web_server_label'] = array(
      '#type' => 'item',
      '#title' => t('Web server'),
      '#markup' => l($server_name, "node/$server_nid", array('attributes' => array('target' => '_blank'))),
    );
  }

  // Force the new environment to use the "default" servers.
  if ($project->settings->default_environment['force_default_servers']) {
    $form['server_stack']['#description'] = t('All environments in this project use this server stack.');

    $web_server_nid = $project->settings->default_environment['web_server'];
    $form['server_stack']['web_server']['#type'] = array(
      '#type' => 'value',
      '#value' => $web_server_nid,
    );
    $form['server_stack']['web_server_label'] = array(
      '#type' => 'item',
      '#title' => t('Web server'),
      '#markup' => l($web_servers[$web_server_nid], "node/$web_server_nid", array('attributes' => array('target' => '_blank'))),
    );

    $db_server_nid = $project->settings->default_environment['db_server'];
    $form['server_stack']['db_server']['#type'] = array(
      '#type' => 'value',
      '#value' => $db_server_nid,
    );
    $form['server_stack']['db_server_label'] = array(
      '#type' => 'item',
      '#title' => t('Web server'),
      '#markup' => l($db_servers[$db_server_nid], "node/$db_server_nid", array('attributes' => array('target' => '_blank'))),
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => '<i class="fa fa-plus"></i> ' . t('Create new Environment'),
    '#submit' => array('node_form_submit'),
  );

  $form['project'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,
  );

  $form['action'] = array(
    '#type' => 'value',
    '#value' => 'create',
  );

  array_unshift($form['#validate'], 'node_form_validate');

  // Add a validator
//  $form['#validate'][] = 'devshop_environment_add_form_validate';
//  $form['#submit'][] = 'devshop_environment_add_form_submit';

  // The rest of the DevShop Environment settings are in devshop_projects_form_site_node_form_alter()
  // so that the site edit node has them as well.

//  dsm($form);
  return $form;
}

/**
 * Validation for devshop_environment_add_form
 * 
 * @param $form
 * @param $form_state
 */
function devshop_environment_add_form_validate($form, &$form_state) {

  $project_node = node_load($form_state['values']['project']);
  $project = $project_node->project;

  // Check existence of the environment name
  if (!empty($project->environments[$form_state['values']['environment_name']])) {
    form_set_error('environment_name', t('There is already an environment named %name in this project. Please choose a different name.', array('%name' => $form_state['values']['environment_name'])));
  }

  // Check for illegal chars
  if (!preg_match('!^[a-z0-9_]+$!', $form_state['values']['environment_name'])) {
    form_set_error('environment_name', t('The environment name must contain only lowercase letters, numbers, and underscores.'));
  }
}


/**
 * Validation for devshop_environment_add_form
 *
 * @param $form
 * @param $form_state
 */
function devshop_environment_add_form_submit($form, &$form_state) {
  $project_node = node_load($form_state['values']['project']);
  $environment_name = $form_state['values']['environment_name'];
  $environment_settings = $form_state['values']['environment']['settings'];
  $environment_settings['install_method'] = $form_state['values']['install_method'];
  $environment_settings['db_server'] = $form_state['values']['db_server'];
  $environment_settings['web_server'] = $form_state['values']['web_server'];
  $environment_settings['deploy'] = $project_node->project->settings->deploy['default_hooks'];

  $action = $form_state['values']['action'];
  $git_ref = $form_state['values']['git_ref'];
  $environment_to_clone = $form_state['values']['environment_source'];

  $site_node_base = (object) $form_state['values'];
  hosting_create_environment($project_node->project, $environment_name, $git_ref, $environment_to_clone, $environment_settings, $action, $site_node_base);

  // Redirect to project dashboard.
  $form_state['redirect'] = 'node/' . $project_node->nid;

  // Friendly message
//  drupal_get_messages('status');
  drupal_set_message(t('Your environment is being created.'));

  // Queue a project verify.
  hosting_add_task($project_node->nid, 'verify');
}
