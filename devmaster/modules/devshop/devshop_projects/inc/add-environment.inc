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

//
//  $output['install'] = array(
//    '#type' => 'fieldset',
//    '#title' => t('Install Profile'),
//    '#description' => t('Install Drupal from scratch using an install profile.'),
//    '#group' => 'add_options',
//    '#collapsible' => TRUE,
//    '#collapsed' => FALSE,
//  );
//  $output['install']['profile_options'] = array(
//    '#theme' => 'item_list',
//    '#items' => array(
//      l(t('Standard'), "hosting_confirm/{$node->nid}/project_devshop-create")
//    ),
//  );
//  $output['clone'] = array(
//    '#type' => 'fieldset',
//    '#title' => t('Clone Environment'),
//    '#description' => t('Clone one of your DevShop environments.'),
//    '#group' => 'add_options',
//    '#collapsible' => TRUE,
//    '#collapsed' => FALSE,
//  );
//  $output['clone']['clone_sources'] = array(
//    '#theme' => 'item_list',
//    '#items' => array(
//      l(t('dev'), "hosting_confirm/{$node->nid}/project_devshop-create?clone_nid=")
//    ),
//  );
//  $output['import'] = array(
//    '#type' => 'fieldset',
//    '#title' => t('Import Data'),
//    '#description' => t('Import existing database and files.'),
//    '#group' => 'add_options',
//    '#collapsible' => TRUE,
//    '#collapsed' => FALSE,
//  );
//  $output['import']['import_sources'] = array(
//    '#theme' => 'item_list',
//    '#items' => array(
//      l(t('dev'), "hosting_confirm/{$node->nid}/project_devshop-create?clone_nid=")
//    ),
//  );
//
//  $output['add_options'] = array(
//    '#type' => 'vertical_tabs',
//  );

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
  $project = $project_node->project;

  // We are creating our own form, but we want to let other modules alter it as
  // if it were the site_node_form. This is how drupal_prepare_form() invokes
  // hook_form_alter() and hook_form_FORM_ID_alter() hooks.
  $form = array();
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
    '#element_validate' => array(
      'devshop_environment_add_form_validate_name'
    ),
  );

  $form['install_method'] = array(
    '#type' => 'radios',
    '#title' => t('Install Method'),
    '#process' => array('devshop_environment_method_process'),
    '#weight' => -99,
    '#options' => array(
      'profile' => t('Drupal Profile'),
      'clone' => t('Clone Environment'),
      'clone_alias' => t('Sync with Drush'),
      'import' => t('Import Database'),
    ),
  );

  $form['install_method_profile'] = array(
    '#type' => 'radios',
    '#title' => t('Drupal Install Profile'),
    '#weight' => -98,
    '#options' => array(
      'minimal' => t('Minimal'),
      'standard' => t('Standard') . ' ' . t('(Project default)'),
      'other' => t('Other'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="install_method"]' => array('value' => 'profile'),
      ),
    ),
  );
  $form['install_method_profile_other'] = array(
    '#type' => 'textfield',
    '#attributes' => array(
      'placeholder' => t('Profile Name'),
    ),
    '#weight' => -97,
    '#states' => array(
      'visible' => array(
        ':input[name="install_method_profile"]' => array('value' => 'other'),
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

  // The rest of the DevShop Environment settings are in devshop_projects_form_site_node_form_alter()
  // so that the site edit node has them as well.

  return $form;
}

function devshop_environment_method_process($element) {
  $element = form_process_radios($element);

  foreach (element_children($element) as $i) {
    $element[$i]['#label_attributes']['class'] = array(
      'btn btn-default'
    );
  }

  kpr($element);
  return $element;
}

/**
 * Element Validator for the domain name field: Sets domain according to pattern
 * and environment name.
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_environment_add_form_validate_name($element, &$form_state, $form) {
  if (!empty($element['#value'])) {

    // Generate field prefix and suffix using domain name pattern.
    $pattern = variable_get('devshop_project_environment_url_pattern', '@environment.@project.@hostname');
    $project = node_load($form_state['values']['project']);
    $domain = strtr($pattern, array(
      '@environment' => $element['#value'],
      '@project' => $project->title,
      '@hostname' => $_SERVER['SERVER_NAME'],
    ));
    form_set_value($form['title'], $domain, $form_state);
  }
}