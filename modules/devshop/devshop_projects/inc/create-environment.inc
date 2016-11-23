<?php

/**
 * Implements hook_form_alter() for Site Creation nodes.
 *
 * @see devshop_projects_form_alter()
 *
 */
function devshop_projects_create_environment_form(&$form, &$form_state, $form_id) {

  // Look for project. If there is none, return.
  $project_node = devshop_projects_load_by_name(arg(3));
  if ($project_node->type != 'project') {
    return;
  }
  $project = $project_node->project;

  drupal_set_title(t('Create new Environment'));

  // Wrap the form in a container so we can use grid
  $form['#prefix'] = '<div class="container add-environment-container">';
  $form['#suffix'] = '</div>';

  // Generate field prefix and suffix using domain name pattern.
  $pattern = variable_get('devshop_project_environment_url_pattern', '@environment.@project.@hostname');

  $labels = explode('@environment', strtr($pattern, array(
    '@project' => $project_node->title,
    '@hostname' => $_SERVER['SERVER_NAME'],
  )));

  // Hide the "Domain Name" field, as this will be generated
  $form['title']['#access'] = false;
  $form['title']['#required'] = false;

  // Add a validator so we can set extra values on the node.
  $form['#validate'][] = 'devshop_projects_create_environment_form_validate';

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
      'devshop_projects_create_environment_form_validate_name'
    ),
    '#wrapper_attributes' => array(
      'class' => array('col-sm-8 col-md-8'),
    ),
  );

  $branch_options = devshop_projects_git_ref_options($project);
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

  $form['project'] = array(
    '#type' => 'value',
    '#value' => $project_node->nid,
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
}

/**
 * Validator for the domain name field: Sets domain according to pattern.
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_projects_create_environment_form_validate_name($element, &$form_state, $form) {
  if (!empty($element['#value'])) {
    $project = node_load($form_state['values']['project']);

    // Check existence of the environment name
    if (!empty($project->environments[$element['#value']])) {
      form_set_error('environment_name', t('There is already an environment named %name in this project. Please choose a different name.', array('%name' => $form_state['values']['environment_name'])));
    }

    // Check for illegal chars
    if (!preg_match('!^[a-z0-9_]+$!', $element['#value'])) {
      form_set_error('environment_name', t('The environment name must contain only lowercase letters, numbers, and underscores.'));
    }

    // Generate field prefix and suffix using domain name pattern.
    $pattern = variable_get('devshop_project_environment_url_pattern', '@environment.@project.@hostname');
    $domain = strtr($pattern, array(
      '@environment' => $element['#value'],
      '@project' => $project->title,
      '@hostname' => $_SERVER['SERVER_NAME'],
    ));
    form_set_value($form['title'], $domain, $form_state);
  }
}