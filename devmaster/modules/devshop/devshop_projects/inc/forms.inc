<?php
/**
 * @file devshop_project.form.inc DevShop Project forms related hooks and
 * support function include file.
 */

/**
 * Implementation of hook_form_FORM_ID_alter().
 *
 * The Project Node Form.
 */
function devshop_projects_form_project_node_form_alter(&$form, &$form_state, $form_id) {
  $node = $form['#node'];
  $project = $node->project;

  // Save last project data
  $form['old'] = array(
    '#value' => $node,
    '#type' => 'value',
  );

  // Project Settings
  // Every value under $form['project'] gets serialized and saved into a project's "data" column.
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
  $form['project']['codebase'] = array(
    '#type' => 'fieldset',
    '#title' => t('General Settings'),
    '#group' => 'project_settings',
    '#tree' => FALSE,
    '#weight' => -11,
  );

  $form['project']['codebase']['status'] = $form['options']['status'];
  unset($form['options']);

  $form['project']['codebase']['git_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Git Repository URL'),
    '#description' => t('The SSH or HTTP url for the git repository to use for new environments. Changing this will not change the git remote for existing environments.'),
    '#default_value' => $project->git_url,
    '#parents' => array(
      'project',
      'git_url',
    ),
  );
  $form['project']['codebase']['code_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Base path'),
    '#description' => t('All environments in this project will be installed inside this path. <em>Changing this will only affect new environments. Existing environments will not change.</em>'),
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->code_path,
    '#maxlength' => 255,
    '#weight' => 2,
    '#parents' => array(
      'project',
      'code_path',
    ),
  );

  $items = [];
  $items[] = t('Enter the relative path to the exposed document root within your repository. Leave blank if index.php is in the root. Common paths are docroot or web.');
  $items[] = t('If you are using the drupal-composer project for Drupal 8, the default document root is "web".');

  $form['project']['codebase']['drupal_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Document Root'),
    '#description' => theme('item_list', ['items' => $items]),
    '#size' => 40,
    '#default_value' => $project->drupal_path,
    '#maxlength' => 255,
    '#weight' => 3,
    '#parents' => array(
      'project',
      'drupal_path',
    ),
  );
// This was accidentally merged from the makefiles branch.
//  $form['project']['codebase']['makefile_path'] = array(
//    '#type' => 'textfield',
//    '#title' => t('Path to Makefile'),
//    '#description' => t('Enter the relative path to a makefile inside your repository. Leave blank if a full Drupal codebasee is in the repository.'),
//    '#size' => 40,
//    '#default_value' => $project->settings->makefile_path,
//    '#maxlength' => 255,
//    '#weight' => 4,
//    '#parents' => array(
//      'project',
//      'settings',
//      'makefile_path',
//    ),
//  );
  $form['project']['codebase']['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Environment Domain Name Pattern'),
    '#description' => t("Each environment will have a system domain name generated for it based on it's name. Use @project for project name, @hostname for '%host', @environment for the environment's name.", array('%host' => $_SERVER['SERVER_NAME'])) . '<em>' . t('Changing this will only affect new environments.'). '</em>',
    '#required' => TRUE,
    '#size' => 40,
    '#default_value' => $project->base_url,
    '#maxlength' => 255,
    '#weight' => 4,
    '#access' => variable_get('devshop_projects_allow_custom_base_url', FALSE),
    '#parents' => array(
      'project',
      'base_url',
    ),
  );

  // Don't allow editing
  if ($node->nid) {

    // Title
    $form['title']['#value'] = $form['title']['#default_value'];
    $form['title']['#type'] = 'value';

  }

  // Prevent editing code path unless allowed.
  if (!variable_get('devshop_projects_allow_custom_code_path', FALSE)) {
    $form['project']['codebase']['code_path']['#type'] = 'value';
    $form['project']['codebase']['code_path']['#value'] = $form['project']['codebase']['code_path']['#default_value'];
  }

  // Prevent editing git url path unless allowed.
  if (!variable_get('devshop_projects_allow_changing_project_git_url', TRUE)) {
    $form['project']['codebase']['git_url']['#type'] = 'value';
    $form['project']['codebase']['git_url']['#value'] = $form['project']['codebase']['git_url']['#default_value'];
  }

  // Project Settings
  $form['project']['settings'] = array(
    '#weight' => -10,
  );

  // Project Settings Vertical Tabs
  $form['project_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => -11,
  );

  // Save git branches and tags
  $form['project']['settings']['git']['branches'] = array(
    '#type' => 'value',
    '#value' => isset($project->settings->git['branches']) ? $project->settings->git['branches'] : NULL,
  );
  $form['project']['settings']['git']['tags'] = array(
    '#type' => 'value',
    '#value' => isset($project->settings->git['tags']) ? $project->settings->git['tags'] : NULL,
  );

  // Live Environment settings.
  $form['project']['settings']['live'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => arg(1) != $project->nid,
    '#title' => t('Domain Name Settings'),
    '#group' => 'project_settings',
  );

  // Live Environment
  $environments = array_keys($project->environments);
  if (empty($environments)) {
    $environments_options = array();
  }
  else {
    $environments_options = array_combine($environments, $environments);
  }

  $form_state['environments'] = $environments_options;
  $environments_options[''] = t('None');

  $form['project']['codebase']['live_environment'] = array(
    '#type' => 'select',
    '#title' => t('Primary Environment'),
    '#suffix' => '<p class="muted">' . t('Select the primary environment for this project, typically the live environment. This environment will be marked with a <i class="fa fa-bolt"></i> icon. This is the environment that will be cloned when pull requests are created.') . '</p>',
    '#options' => $environments_options,
    '#default_value' => isset($project->settings->live) ? $project->settings->live['live_environment'] : '',
    '#parents' => array(
      'project',
      'settings',
      'live',
      'live_environment',
    ),
  );

  // Live Domain
  $form['project']['settings']['live']['live_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('Live Domain'),
    '#description' => t('The live domain for this project. Used only for links and when creating subdomain aliases for other environments. <em>You must still add the Domain to your live environment manually.</em>'),
    '#size' => 40,
    '#default_value' => isset($project->settings->live) ? $node->project->settings->live['live_domain'] : '',
  );

  // Use aliases
  $form['project']['settings']['live']['environment_aliases'] = array(
    '#type' => 'checkbox',
    '#title' => t('For new environments, create subdomains under Live Domain.'),
    '#description' => t('When new environments are created, automatically add a domain name such as http://ENVIRONMENT.LIVEDOMAIN.com. <em>Does not affect existing environments. Does not remove domains when disabled.</em>'),
    '#default_value' => isset($project->settings->live) ? $project->settings->live['environment_aliases'] : FALSE,
  );

  // Pull Code Method
  $form['project']['settings']['deploy'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => arg(1) != $project->nid,
    '#title' => t('Deployment Automation'),
    '#description' => t('Configure how code is delivered to the servers. Post Deploy hooks are configured per environment.'),
    '#weight' => -9,
    '#group' => 'project_settings',
  );

  $form['project']['settings']['deploy']['method'] = array(
    '#title' => 'Deploy Code Method',
    '#type' => 'radios',
    '#description' => t('Choose the method used to deploy code to the server.'),
    '#default_value' => isset($project->settings->deploy['method']) ? $project->settings->deploy['method'] : 'webhook',
  );

  // Commit Webhook
  $form['project']['settings']['deploy']['method']['#options']['webhook'] = t('Immediate Deployment');
  $form['project']['settings']['deploy']['method']['#options']['webhook'] .= '<div class="description">' . t('Recommended. Deploy code as it is delivered to your repository.') . ' <em>' . t('Requires setting up a webhook with your git repository host.') . '</em>' . '</div>';

  // Queue
  $queues = hosting_get_queues();
  if ($queues['pull']['enabled']) {
    $form['project']['settings']['deploy']['method']['#options']['queue'] = t('Queued Deployment');

    $t = array();
    $t['@freq'] = format_interval($queues['pull']['frequency'], 1);

    $form['project']['settings']['deploy']['method']['#options']['queue'] .= '<div class="description">';
    $form['project']['settings']['deploy']['method']['#options']['queue'] .= t('Git pull code every @freq. Use only if repository webhooks are not available.', $t);


    if (user_access('administer hosting queues')) {
      $form['project']['settings']['deploy']['method']['#options']['queue'] .= ' ' . l(t("Pull Queue configured to run every @freq.", $t), 'admin/hosting/queues');
    }
    else {
      $form['project']['settings']['deploy']['method']['#options']['queue'] .= ' ' . t("Pull Queue configured to run every @freq.", $t);
    }

    $form['project']['settings']['deploy']['method']['#options']['queue'] .= '</div>';

  }

  // Manual Pull
  $form['project']['settings']['deploy']['method']['#options']['manual'] = t('Manual Deployment');
  $form['project']['settings']['deploy']['method']['#options']['manual'] .= '<div class="description">' . t('Deploy code to servers manually via devshop or drush.');

  $form['project']['settings']['deploy']['method']['#options']['manual'] .= ' <em>' . t('Not recommended. All environments must be manually updated.') . '</em>' . '</div>';

  // Add link to hosting queues admin if the user can access them.
  if (!isset($queues['deploy']['enabled']) && user_access('administer hosting queues')) {
    $form['project']['settings']['deploy']['queue_admin'] = array(
      '#value' => t('The !link is disabled. Enable it to allow projects to pull code in the queue.', array(
        '!link' => l(t('Pull Queue'), 'admin/hosting/queues'),
      )),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    );
  }

  // Load deploy hooks form element.
  $form['project']['settings']['deploy']['default_hooks'] = devshop_environment_deploy_hooks_form($project);

  // Deploy hooks configuration
  $form['project']['settings']['deploy']['default_hooks']['label'] = array(
    '#value' => t('Default Deploy Hooks:'),
    '#prefix' => '<label>',
    '#suffix' => '</label>',
    '#weight' => -100,
  );
  $form['project']['settings']['deploy']['default_hooks']['description'] = array(
    '#value' => t('New environments will run these actions when new code or data is deployed.'),
    '#prefix' => '<p>',
    '#suffix' => '</p>',
    '#weight' => -100,
  );

  $form['project']['settings']['deploy']['default_hooks']['allow_environment_deploy_config'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow environment-specific deploy hook configuration.'),
    '#default_value' => isset($node->project->settings->deploy) ? $node->project->settings->deploy['allow_environment_deploy_config'] : FALSE,
    '#description' => t('Each environment can be configured to have different deploy hooks. <em>Be sure to check your environments settings if you enable this.</em>' ),
    '#parents' => array(
      'project',
      'settings',
      'deploy',
      'allow_environment_deploy_config',
    ),
  );
  $form['project']['settings']['deploy']['default_hooks']['allow_environment_deploy_hooks_override'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow users to override hooks when deploying manually.'),
    '#default_value' => isset($node->project->settings->deploy) ? $node->project->settings->deploy['allow_environment_deploy_hooks_override'] : FALSE,
    '#description' => t('Check this box to allow users to override the hooks that are run on manual deployments. If left unchecked, all environments will run the deploy hooks configured above on every deployment.' ),
    '#parents' => array(
      'project',
      'settings',
      'deploy',
      'allow_environment_deploy_hooks_override',
    ),
  );

  // @TODO: is there a better way to save certain values? We lose data without these.
  $form['project']['settings']['deploy']['last_webhook'] = array(
    '#type' => 'value',
    '#value' => isset($node->project->settings->deploy) ? $node->project->settings->deploy['last_webhook'] : NULL,
  );
  $form['project']['settings']['deploy']['last_webhook_status'] = array(
    '#type' => 'value',
    '#value' => isset($node->project->settings->deploy) ? $node->project->settings->deploy['last_webhook_status'] : NULL,
  );
  $form['project']['settings']['deploy']['last_webhook_ip'] = array(
    '#type' => 'value',
    '#value' => isset($node->project->settings->deploy) ? $node->project->settings->deploy['last_webhook_ip'] : NULL,
  );

  //All settings git pull in project page
  $form['project']['settings']['default_environment'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => arg(1) != $project->nid,
    '#title' => t('Server Stack'),
    '#description' => t('The servers to use when creating new environments.'),
    '#group' => 'project_settings',
  );

  // Install Profile
  $available_profiles = array();
  foreach ($project->environments as $name => $environment) {
    // Passing null to hosting_get_profiles returns all profiles.
    if (is_null($environment->platform)) {
      continue;
    }
    $profiles_shortnames = hosting_get_profiles($environment->platform, 'short_name');
    if (is_array($profiles_shortnames) && !empty($profiles_shortnames)) {
      $profiles[$name] = array_combine($profiles_shortnames, (array)hosting_get_profiles($environment->platform));
    } else {
      $profiles[$name] = array();
    }
    if (empty($available_profiles)) {
      $available_profiles = $profiles[$name];
    } else {
      $available_profiles = array_intersect_key($available_profiles, $profiles[$name]);
    }
  }

  $form['project']['settings']['default_environment']['install_profile'] = array(
    '#type' => 'radios',
    '#options' => $available_profiles,
    '#title' => t('Default Install Profile'),
    '#required' => 1,
    '#description' => t('New environments will be created using this install profile. Existing environments will not be affected.'),
    '#default_value' => $node->project->install_profile,
    '#access' => count($available_profiles),
  );

  // HTTP Server select.
  $http_servers = hosting_get_servers('http', FALSE);
  if (count($http_servers)) {
    $form['project']['settings']['default_environment']['web_server'] = array(
      '#title' => t('Web server'),
      '#type' => 'select',
      '#options' => $http_servers,
      '#default_value' => isset($project->settings->default_environment) ? $project->settings->default_environment['web_server'] : current($http_servers),
    );
  }

  // DB Server select.
  $db_servers = hosting_get_servers('db', FALSE);
  if (count($db_servers)) {
    $form['project']['settings']['default_environment']['db_server'] = array(
      '#title' => t('Database server'),
      '#type' => 'select',
      '#options' => $db_servers,
      '#default_value' => isset($project->settings->default_environment) ? $project->settings->default_environment['db_server'] : current($db_servers),
    );
  }

  // Solr Server Select
  $solr_servers = hosting_get_servers('solr', FALSE);
  if (count($solr_servers)) {
    $form['project']['settings']['default_environment']['solr_server'] = array(
      '#title' => t('Solr server'),
      '#type' => 'select',
      '#options' => $solr_servers,
      '#default_value' => $project->settings->default_environment['solr_server'],
    );
  }

  // Force the servers for this project's environments
  $form['project']['settings']['default_environment']['force_default_servers'] = array(
    '#title' => t('Force new environments to use these servers.'),
    '#description' => t('When new environments are created, they will use the servers selected above.'),
    '#type' => 'checkbox',
    '#default_value' => $project->settings->default_environment['force_default_servers']?: 0,
  );
  return $form;
}

/**
 * Implementation of hook_validate().
 *
 * This function is no longer used since we have a ctools wizard for
 * project creation.
 */
function devshop_projects_validate($node, &$form, &$form_state) {

  // Code path and base url must be unique
  if (db_query('SELECT n.nid FROM {hosting_devshop_project} d LEFT JOIN {node} n ON d.nid = n.nid WHERE status = :status AND code_path = :code_path AND n.nid != :nid;', array(':status' => 1, ':code_path' => $form_state['values']['project']['code_path'], ':nid' => $form_state['values']['nid']))->fetchField()) {
    form_set_error('code_path', t('The code path %path is in use by another project. Please enter a different path.', array(
      '%path' => $form_state['values']['project']['code_path'],
    )));
  }
  // If custom domain pattern is allowed, AND domain pattern does not have "@project" in it, AND domain pattern is not the default AND it is found on another project, throw an error.
  if (variable_get('devshop_projects_allow_custom_base_url', FALSE) && strpos($form_state['values']['project']['base_url'], '@project') === FALSE && $form_state['values']['project']['base_url'] !=  variable_get('devshop_project_environment_url_pattern', '@project.@environment.@hostname') && db_query('SELECT n.nid FROM {hosting_devshop_project} d LEFT JOIN {node} n ON d.nid = n.nid WHERE status = :status AND base_url = :base_url AND n.nid != :nid;;', array(':status' => 1, ':base_url' => $form_state['values']['project']['base_url'], ':nid' => $form_state['values']['nid']))->fetchField()) {
    form_set_error('base_url', t('The Environment Domain Pattern %url is in use by another project.  Please enter a different Environment Domain Pattern.', array(
      '%url' => $form_state['values']['project']['base_url'],
    )));
  }

  // "Environment Domain Name Pattern" (Base URL)
  // Ensure @environment is included.
  if (strpos($form_state['values']['project']['base_url'], '@environment') === FALSE ) {
    form_set_error('project][base_url', t('Environment Domain Name Pattern must include @environment placeholder.'));
  }
}

/**
 * Implements hook_form_alter().
 */
function devshop_projects_form_alter(&$form, &$form_state, $form_id) {

  // Removing unneccesary fieldgroups
  if ($form_id == 'project_node_form') {
    global $user;

    $project = $form['#node'];
    unset($form['menu']);
    unset($form['revision_information']);
    unset($form['author']);

    // Move Published checkbox to General Settings group.
    $form['options']['status']['#group'] = 'codebase';
    $form['options']['status']['#title'] = t('Active');
    $form['options']['status']['#description'] = t('Uncheck this box to hide the project from the main dashboard. Users who have permission to "view unpublished nodes" will still be able to view inactive projects.');

    // On "project/add/settings" page, don't show the checkbox.
    if (current_path() == 'projects/add/settings') {
      $form['options']['status']['#access'] = 0;
    }

    unset($form['actions']['delete']);
    unset($form['actions']['preview']);
    if (isset($form['retry']['#value'])) {
      $form['actions']['submit']['#value'] = t('Save and Retry');
    }

    //Add button for delete project
    $form['actions']['delete'] = array(
      '#markup' => l('<i class="fa fa-trash"></i> ' . t('Delete Project'), 'hosting_confirm/' . $project->nid . '/project_delete', array(
        'query' => array('token' => drupal_get_token($user->uid)),
        'attributes' => array(
          'class' => array('btn btn-danger pull-right'),
        ),
        'html' => TRUE,
      )),
      '#weight' => 10,
      '#access' => drupal_valid_path('hosting_confirm/' . $project->nid . '/project_delete'),
    );
  }

  // Create Project Wizard
  if (
    $form_id == 'devshop_project_create_step_git' ||
    $form_id == 'devshop_project_create_step_sites' ||
    $form_id == 'devshop_project_create_step_settings' ||
    $form_id == 'devshop_project_create_step_environments'
  ){
    if (isset($form['#no_finish'])) {
      unset($form['buttons']['return']);
    }
    if (isset($form['#no_next'])) {
      unset($form['buttons']['next']);
    }

    if (isset($form['buttons']['next']['#value'])) {
      $form['buttons']['next']['#value'] .= ' <i class="fa fa-chevron-right"></i>';
    }

    if (isset($form['buttons']['return']['#value'])) {
      $form['buttons']['return']['#value'] = ' <i class="fa fa-rocket"></i> ' . t('Create Project & Environments');
    }

    $form['buttons']['next']['#attributes'] =
    $form['buttons']['return']['#attributes'] = array(
      'class' => array(
          'btn btn-success',
      ),
    );
    $form['buttons']['previous']['#attributes'] = array(
      'class' => array(
          'btn btn-default',
      ),
    );
    $form['buttons']['cancel']['#attributes'] = array(
      'class' => array(
          'btn btn-link',
      ),
    );

  }

  // Hosting Task Forms
  if ($form_id == 'hosting_task_confirm_form') {

    // Allow hosting_task_TASK_TYPE_form to define their page title.
    if (!empty($form['parameters']['#page_title'])) {
      drupal_set_title($form['parameters']['#page_title']);
    }

    switch ($form['task']['#value']) {

      // Migrate Form: used for changing database server.
      case 'migrate':

        // To change the database server, we use the migrate form.
        if ($_GET['deploy'] == 'stack') {
          drupal_set_title(t('Change Database Server'));
          $site_node = node_load($form['nid']['#value']);
          $environment = $site_node->environment;
          $form['help']['#weight'] = 100;
          $form['help']['#value'] = t("Are you sure you want to change this environment's database server?");

          // Hide "URI" field unless "rename" GET parameter is added.
          if (!isset($_GET['rename'])) {
            $form['parameters']['new_uri']['#type'] = 'value';
            $form['parameters']['new_uri']['#value'] = $form['parameters']['new_uri']['#default_value'];
          }

          // Display something helpful
          $form['old'] = array(
            '#type' => 'item',
            '#title' => t('Current Database Server'),
            '#value' => l($environment->servers['db']['name'], 'hosting/c/server_' . $environment->servers['db']['nid']),
            '#weight' => '-1',
          );
        }
        if (isset($_GET['rename'])) {
          drupal_set_title(t('Change Domain Name for environment @env in project @proj?', array(
            '@env' => $environment->name,
            '@proj' => $environment->project_name,
          )));
          $form['help']['#value'] = t("Are you sure you want to change this environment's domain name?");
          $form['submit']['#value'] = t('Change Domain');
        }
        if (isset($_GET['rename']) || $_GET['deploy'] == 'stack') {
          foreach ($form['parameters'] as $key => $element) {
            if (is_int($key)) {
              // Don't unset the target platform. Just hide it.
              if (isset($element['target_platform']['#default_value'])) {
                $form['parameters'][$key]['target_platform']['#type'] = 'value';
                $form['parameters'][$key]['target_platform']['#value'] = $form['parameters'][$key]['target_platform']['#default_value'];
              }
              else {
                unset($form['parameters'][$key]);
              }
            }
          }
        }
        break;

      // Deploy task form
      case 'deploy':

        // Alter title of deploy task.
        $node = node_load($form['nid']['#value']);
//        drupal_set_title(t('Deploy code to Environment "@env"', array('@env' => $node->environment->name)), PASS_THROUGH);
//        $form['actions']['cancel']['#value'] = l(t('Cancel'), "node/{$node->project->nid}");
        break;

      // Sync task form
      case 'sync':
        if (isset($_GET['source'])) {
          $node = node_load($form['nid']['#value']);

          if ($_GET['source'] == 'other') {
            $source = '';
          }
          elseif (is_int($_GET['source'])) {
            $source = node_load($_GET['source']);
            if (isset($source->nid) && $source->type != 'site') {
              break;
            }
              $source_alias = $source->environment->system_alias;
          }
          elseif (isset($node->project->settings->aliases[$_GET['source']])) {
              $alias_name = $source = $_GET['source'];
              $source_alias = "@{$node->project->name}.{$alias_name}";
              $alias_data = $node->project->settings->aliases[$_GET['source']];
          }
          else {
              return;
          }

//          print_r($source); die;
//          print $source_alias ; die;

          drupal_set_title(t('Sync Data to Environment "@env"', array('@env' => $node->environment->name)), PASS_THROUGH);
          $environment = $node->environment;

          // If source is empty, ask for an alias.
          if (empty($source)) {
            $form['parameters']['source'] = array(
              '#type' => 'textfield',
              '#title' => t('Drush Alias for Source'),
              '#description' => t('Enter a full drush alias to use as the source. Make sure to include the "@" character at the beginning.'),
            );
            $form['parameters']['destination'] = array(
              '#type' => 'value',
              '#value' => $node->environment->system_alias,
            );
          }
          // If a source is specified, add help and set it as a value.
          else {

            $source_environment = $source->environment;
            $form['parameters']['source'] = array(
              '#type' => 'value',
              '#value' => $source_alias,
            );
            $form['parameters']['destination'] = array(
              '#type' => 'value',
              '#value' => $node->environment->system_alias,
            );
          }

          // Don't copy modules and themes.
          unset($form['parameters']['modules']);
          unset($form['parameters']['themes']);
          unset($form['parameters']['libraries']);

          // Better output
          $form['parameters']['database']['#prefix'] = '<label>' . t('What to Sync?') . '</label>';
          $form['parameters']['registry-rebuild']['#prefix'] = '<label>' . t('Run Deploy Hooks?') . '</label>';
          $form['parameters']['backup']['#weight'] = -11;

          $form['actions']['submit']['#value'] = '<i class="fa fa-arrow-right"></i>' . t('Sync Data');
          $form['actions']['submit']['#attributes'] = array(
            'class' => array(
              'btn-success'
            ),
          );
          $form['actions']['cancel']['#value'] = l(t('Cancel'), "node/{$node->project->nid}");

          $form['warning']['#markup'] = '<div class="alert alert-spacing alert-danger">' . t('Clicking "Sync Data" will DESTROY the database for the environment !link.', array('!link' => l($environment->url, $environment->url, array('attributes' => array('target' => '_blank'))))) . '</div>';


          $destroyed = t('This environment will be destroyed.');
          $source = t('This environment will be copied.');

          $source_environment_url = $source_environment->url? $source_environment->url: 'http://' . $alias_data['uri'];
          $source_environment_name = $source_environment->name? $source_environment->name: $alias_name;
          $html = <<<HTML
          
<div class="container no-wrap">
  <div class="inline">
    <a href="$source_environment_url" target="_blank" class="btn btn-default btn-lg"><span class="text-lg"><i class="fa fa-globe"></i> {$source_environment_name} </span>
      <br/>   <small>$source_environment_url</small>
    </a>
    <br/>   <em class="text-sm">$source</em>

  </div>
  <div class="inline btn">
  
    <i class="fa fa-arrow-right fa-4x"></i>
    
  </div>
  <div class="inline">
    <a href="$environment->url" target="_blank" class="btn btn-danger btn-lg"><span class="text-lg"><i class="fa fa-trash"></i> {$environment->name} </span>
      <br/>   <small class="text-sm">$environment->url</small>
    </a>
    <br/>   <em class="text-sm">$destroyed</em>
  </div>
</div>
HTML;

          $form['notes'] = array(
            '#markup' => $html,
            '#weight' => -10,
          );
        }
        break;
      case 'delete':
        $node = node_load($form['nid']['#value']);
        if ($node->type == 'project') {

          // Set a better title.
          drupal_set_title('');
          $form['title'] = array(
            '#weight' => -10,
            '#markup' => '<h1>' .t('Delete Project: !project', array(
            '!project' => l($node->project->name, "node/{$node->nid}"),
          )) . '</h1>'
          );

          // Get a list of environments.
          foreach ($node->project->environments as $environment) {
            $items[] = l($environment->uri, 'http://' . $environment->uri);

          }
          $environments = theme('item_list', array('items' => $items));

          // Display a scary warning message.
          $form['help']['#markup'] = '<div class="alert alert-spacing alert-danger">' . t('Are you sure you wish to destroy the project !project and all of these environments?', array(
              '!project' => l($node->project->name, "node/{$node->nid}"),
            )) . $environments . '</div>';

          // Make the button big
          $form['actions']['submit']['#value'] = t('Delete Project & Environments');
          $form['actions']['submit']['#attributes']['class'][] = 'btn btn-large btn-danger';

        }
        break;
    }
  }

  // Platform "Edit" page.
  if ($form_id == 'platform_node_form') {

    // Set to values
    $platform_node = node_load($form['nid']['#value']);
    if (empty($platform_node->project)) {
      return;
    }

    $form['title']['#type'] = 'value';
    unset($form['info']);

    if (isset($platform_node->project)) {
      // Hide makefile
      $form['frommakefile']['#access'] = FALSE;
      $form['git']['pull_url']['#access'] = FALSE;
    }

    // If switching webservers...
    if ($_GET['web_server']) {
      $environment = $platform_node->environment;
      $form['help'] = array(
        '#markup' => t("Are you sure you want to change this site's web server?  <strong>NOTE: You will have to change DNS as well!</strong>"),
        '#weight' => -10,
      );

      $web_server = $_GET['web_server'];
      $web_server_nid = db_query('SELECT nid FROM {node} WHERE type = :type && title = :title', array(':type' => "server", ':title' => $web_server))->fetchField();

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
        '#markup' => l($environment->servers['http']['name'], 'node/' . $environment->servers['http']['nid']) . '<br />' . implode($old_server->ip_addresses, '<br />'),
        '#weight' => '-1',
      );

      // Display something helpful
      $form['new'] = array(
        '#type' => 'item',
        '#title' => t('New Web Server'),
        '#markup' => l($web_server, "node/$web_server_nid") . '<br />' . implode($new_server->ip_addresses, '<br />'),
        '#weight' => '0',
      );
    }
  }
}

/**
 * Implements hook_form_FORM_ID_alter() for node_site_form
 *
 * "Environment" Settings form.
 */
function devshop_projects_form_site_node_form_alter(&$form, &$form_state, $form_id) {

  $node = $form['#node'];

  $form['environment'] = array(
    '#weight' => -9,
    '#tree' => true,
  );

  // On site create page, use devshop_projects_create_environment_form()
  if (arg(1) == 'add') {
    devshop_projects_create_environment_form($form, $form_state, $form_id);

    $project = devshop_projects_load_by_name(arg(3));
    $form_state['project'] = $project->project;
  }

  // Only act on site nodes that are in projects.
  elseif (!isset($node->environment)) {
    return;
  }
  else {

    // Get environment object.  (Convert if it is an array. This happens when clicking "Add an Alias".
    $environment = (object) $node->environment;
    $project = $node->project;

    // Remove aegir's helpful info panel.
    unset($form['info']);

    // Values
    $form['environment']['project_nid'] = array(
      '#value' => $environment->project_nid,
      '#type' => 'value',
    );
    $form['environment']['project_name'] = array(
      '#value' => $environment->project_name,
      '#type' => 'value',
    );
    $form['environment']['name'] = array(
      '#value' => $environment->name,
      '#type' => 'value',
    );
    $form['environment']['git_ref'] = array(
      '#value' => $environment->git_ref,
      '#type' => 'value',
    );
    $form['environment']['site'] = array(
      '#value' => $environment->site,
      '#type' => 'value',
    );
    $form['environment']['platform'] = array(
      '#value' => $environment->platform,
      '#type' => 'value',
    );

    $form['environment']['last_task'] = array(
      '#type' => 'value',
      '#value' => $environment->last_task,
    );
    $form['form_title'] = array(
      '#markup' => t('Environment Settings'),
      '#prefix' => '<h3>',
      '#suffix' => '</h3>',
      '#weight' => -1000,
    );
  }

  // User-configurable Settings
  $form['environment']['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Environment Settings'),
      '#tree' => true,
  );
  $form['environment']['settings']['locked'] = array(
      '#type' => 'checkbox',
      '#title' => t('Protect Site'),
      '#default_value' => isset($environment->settings->locked) ? $environment->settings->locked : FALSE,
      '#description' => t('Prevent devshop users from destroying the site.') . '<p class="text-danger">'. t('Users with server access can still destroy sites and data.') . '</p>',
  );

  $form['environment']['settings']['pull_disabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Disable Deploy on Commit'),
      '#default_value' => isset($environment->settings->pull_disabled) ? $environment->settings->pull_disabled : FALSE,
      '#description' => t('Do not pull code to the server on commit & push.'),
  );

  // Load deploy hooks form element.
  $form['environment']['settings']['deploy'] = devshop_environment_deploy_hooks_form($project, $environment);

  // UX Improvements
  // Project Settings Vertical Tabs
  $form['environment_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => -11,
  );

  $form['environment']['settings']['#title'] = t('General Settings');
  $form['environment']['settings']['#group'] = 'environment_settings';
  $form['environment']['settings']['#weight'] = -100;
  $form['environment']['settings']['deploy']['#group'] = 'environment_settings';
  $form['hosting_backup_queue']['#group'] = 'environment_settings';
  $form['http_basic_auth']['#group'] = 'environment_settings';
  $form['http_basic_auth']['#title'] = t('Password Protection');
  $form['hosting_logs']['#group'] = 'environment_settings';

  // Have to add new element for Domains because of the way drupal ajax callback works.
  $form['domains'] = array(
    '#title' => t('Domain Names'),
    '#type' => 'fieldset',
    '#group' => 'environment_settings',
  );
  $form['domains']['aliases_wrapper'] = $form['aliases_wrapper'];
  $form['domains']['aliases_wrapper']['#type'] = 'container';
  $form['domains']['aliases_wrapper']['add_alias']['#ajax']['callback'] = 'devshop_hosting_alias_add_alias_callback';
  unset($form['aliases_wrapper']);

  $form['environment']['settings']['client'] = $form['client'];
  $form['environment']['settings']['client']['#tree'] = FALSE;
  unset($form['client']);

  $form['environment']['settings']['site_language'] = $form['site_language'];
  $form['environment']['settings']['site_language']['#tree'] = FALSE;
  unset($form['site_language']);

  if (isset($form['hosting_ssl_wrapper'])) {
    $form['hosting_ssl_wrapper']['#group'] = 'environment_settings';
    $form['hosting_ssl_wrapper']['#title'] = t('HTTPS');
    $form['hosting_ssl_wrapper']['#prefix'] = '';
    $form['hosting_ssl_wrapper']['#suffix'] = '';
  }
  if (isset($form['hosting_https_wrapper'])) {
    $form['hosting_https_wrapper']['#group'] = 'environment_settings';
    $form['hosting_https_wrapper']['#title'] = t('HTTPS');
    $form['hosting_https_wrapper']['#prefix'] = '';
    $form['hosting_https_wrapper']['#suffix'] = '';
  }

  // Ensure values are saved.
  if (!isset($form['install_method'])) {
    $form['install_method'] = array(
      '#type' => 'value',
      '#value' => $environment->settings->install_method,
    );
  }

  // Add our own submit handler.
  $form['actions']['submit']['#submit'][] = 'devshop_projects_environment_settings_submit';

}


/**
 * Ajax callback for returning "Add Alias" form element, since we moved ours to a sub fieldset.
 */
function devshop_hosting_alias_add_alias_callback($form, $form_state) {
  return $form['domains']['aliases_wrapper'];
}


/**
 * Form alters for "site_node_form", which is our "Create Environment" form.
 *
 * Code that only applies to "site create" form should go here. If it needs to
 * be in create and edit, it should go in devshop_projects_form_site_node_form_alter()
 *
 * This is invoked from devshop_projects_form_site_node_form_alter()
 *
 * @see devshop_projects_form_site_node_form_alter()
 *
 */
function devshop_projects_create_environment_form(&$form, &$form_state, $form_id) {

  // Look for project. If there is none, return.
  $project_node = devshop_projects_load_by_name(arg(3));
  if ($project_node->type != 'project') {
    return;
  }
  $project = $project_node->project;

  if ($project_node->status != NODE_PUBLISHED) {
    drupal_set_message(t('This project is not active. You may still create environments but non-admin users will not see the project or sites. Visit !settings to reactivate the project.', array(
      '!settings' => l(t('Project Settings'), "node/{$project->nid}/edit"),
    )), 'warning');
  }
  // Wrap the form in a container so we can use grid
  $form['#prefix'] = '<div class="container add-environment-container">';
  $form['#suffix'] = '</div>';

  $form['form_title'] = array(
    '#markup' => t('Create New Environment'),
    '#prefix' => '<h3>',
    '#suffix' => '</h3>',
    '#weight' => -1000,
  );

  // Add our own validator and submit handlers.
  $form['actions']['submit']['#value'] = t('Create New Environment');
//  array_unshift($form['actions']['submit']['#submit'], 'devshop_projects_create_environment_form_submit');

  // Generate field prefix and suffix using domain name pattern.
  if (variable_get('devshop_projects_allow_custom_base_url')) {
    $pattern = $project->base_url;
  }
  else {
    $pattern = variable_get('devshop_project_environment_url_pattern', '@project.@environment.@hostname');
  }

  $labels = explode('@environment', strtr($pattern, array(
    '@project' => $project_node->title,
    '@hostname' => $_SERVER['SERVER_NAME'],
  )));

  // Hide the "Domain Name" field, as this will be generated
  $form['title']['#access'] = false;
  $form['title']['#required'] = false;

  $form['environment_name'] = array(
    '#title' => t('Environment Name'),
    '#type' => 'textfield',
    '#description' => t('Enter a unique name for your environment. It will be used to generate a domain name. You can add additional domain names later.'),
    '#required' => TRUE,
    '#field_prefix' => '<div class="input-group"><div class="input-group-addon">http://' . $labels[0] . '</div>',
    '#field_suffix' => '<div class="input-group-addon input-group-addon-url">' . $labels[1] .'</div>
        </div>',
    '#size' => 10,
    '#maxlength' => 64,
    '#weight' => -101,
    '#attributes' => array(
      'data-placement' => 'bottom',
      'placeholder' => t('name'),
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
    '#description' => t('The git reference to checkout for this environment. You can change this later using the "Deploy Code" button.'),
    '#options' => $branch_options,
    '#required' => TRUE,
    '#weight' => -100,
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
      'clone' => t('Clone Environment'),
      'profile' => t('Drupal Profile'),
      'manual' => t('Empty Database'),
      'import' => t('Import Database'),
    ),
    '#default_value' => 'clone',
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
    '#description' => t('These install profiles were found in other environments in this project. If you choose an install profile that does not exist in the codebase, the "Standard" profile will be used.'),
    '#description_display' => 'before',
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
  if (count($form['install_method']['profile']['#options']) == 1 || empty($project->install_profile)) {
    $form['install_method']['profile']['#default_value'] = '_other';
  }
  else {
    $form['install_method']['profile']['#default_value'] = $project->install_profile;
  }

  $form['install_method']['profile_other'] = array(
    '#type' => 'textfield',
    '#title' => t('Other Install Profile'),
    '#title_display' => 'invisible',
    '#description' => t('Enter the exact name the install profile to use. It must already exist in your git branch or tag.'),
    '#element_validate' => array(
      'devshop_projects_create_environment_form_validate_profile'
    ),
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
    $link = l(' <i class="fa fa-external-link"></i> ' . $environment->uri,  $environment->url, array(
      'html' => TRUE,
      'attributes' => array(
        'target' => '_blank',
        'class' => array('btn-text'),
      ),
    )) . '</small>';
    $git_ref = $environment->git_ref;
    $icon = $environment->git_ref_type == 'tag'? 'tag': 'code-fork';
    $environment_options[$environment->system_alias] = "<span>{$environment->name}</span> <small class='git-ref text-muted'><i class='fa fa-$icon'></i> $git_ref</small> <small>$link</small>";
  }

  // Add other aliases to the clone source.
  if (isset($project->settings->aliases)) {
    foreach ($project->settings->aliases as $alias => $data) {
      $link = l('<i class="fa fa-external-link"></i> ' . $data['uri'], 'http://' . $data['uri'], array(
          'html' => TRUE,
          'attributes' => array(
            'target' => '_blank',
            'class' => array('btn-text'),
          ),
        )) . '</small>';
      $remote = t('Unknown');
      $environment_options["@{$project->name}.{$alias}"] = "<span>{$alias}</span> <small class='git-ref text-muted'><i class='fa fa-globa'></i> $remote</small> <small>$link</small>";
    }
  }

  $environment_options['_other'] = t('Other Drush Alias');

  if (count($environment_options)) {
    $form['install_method']['clone_source'] = array(
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

  // Detect clone link. Set defaults.
  if (arg(4) == 'clone') {
    $environment_to_clone = arg(5);
    $form['install_method']['method']['#default_value'] = 'clone';
    $form['install_method']['clone_source']['#default_value'] = $project->environments[$environment_to_clone]->system_alias;
    $form['git_ref']['#default_value'] = $project->environments[$environment_to_clone]->git_ref;
  }

  // Detect fork link.
  // @TODO: Get "forking" back in place for the next beta.
//  if (arg(4) == 'fork') {
//    $environment_to_clone = arg(5);
//    $form['install_method']['method']['#default_value'] = 'clone';
//    $form['install_method']['clone_source']['#default_value'] = $project->environments[$environment_to_clone]->system_alias;
//    $form['git_ref']['#default_value'] = $project->environments[$environment_to_clone]->git_ref;
//    $form['git_ref']['#title'] = t('Base Branch or Tag');
//    $form['git_ref']['#description'] = t('The git reference you want to create a new branch from.');
//
//    $form['git_ref_new'] = array(
//      '#title' => t('New branch name'),
//      '#description' => t('New branch name'),
//    );
//  }


  $form['install_method']['clone_source_drush'] = array(
    '#type' => 'textfield',
    '#title' => t('Drush Alias'),
    '#description' => t('Enter a Drush alias you would like to Sync your database and files from. It must already exist on the server.'),
    '#element_validate' => array(
      'devshop_projects_create_environment_form_validate_clone_source'
    ),
    '#weight' => -98,
    '#attributes' => array(
      'placeholder' => t('@drush.alias'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[clone_source]"]' => array('value' => '_other'),
        ':input[name="install_method[method]"]' => array('value' => 'clone'),
      ),
    ),
  );

  $form['install_method']['import'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to SQL'),
    '#description' =>
      t('Enter either a remote MySQL address (such as mysql://username:password@host/database), or an absolute path to an SQL dump (such as /var/aegir/site-backup.sql).') .
      '<span class="text-danger">' . t('This string is stored in plain text. Use with caution.') . '</span>'
    ,
    '#weight' => -98,
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[method]"]' => array('value' => 'import'),
      ),
    ),
    '#element_validate' => array(
      'devshop_projects_create_environment_form_validate_import'
    ),
  );

  $form['install_method']['import_files'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to Files Snapshot Directory'),
    '#description' =>
      t('Enter an absolute path to a files snapshot directory (such as /var/aegir/backups/files-snapshot/).') .
      '<span class="text-danger">' . t('This string is stored in plain text. Use with caution.') . '</span>',
    '#weight' => -98,
    '#states' => array(
      'visible' => array(
        ':input[name="install_method[method]"]' => array('value' => 'import'),
      ),
    ),
  );

  $form['server_stack'] = array(
    '#type' => 'fieldset',
    '#title' => t('Server Stack'),
    '#group' => 'environment_settings',
  );

  $form['server_stack']['db_server'] = $form['db_server'];
  $form['server_stack']['db_server']['#group'] = 'environment_settings';
  $form['server_stack']['db_server']['#tree'] = FALSE;
  unset($form['db_server']);

  $web_servers = hosting_get_servers('http', FALSE);
  $form['server_stack']['web_server'] = array(
    '#type' => 'radios',
    '#title' => t('Web server'),
    '#description' => t('The web server the site will be hosted on.'),
    '#options' => $web_servers,
    '#required' => TRUE,
    '#tree' => FALSE,
    '#default_value' => $project->settings->default_environment['web_server'],
  );

  // Force the new environment to use the "default" servers.
  if ($project->settings->default_environment['force_default_servers']) {
    $form['server_stack']['#description'] = t('All environments in this project must use this server stack.');

    $web_server_nid = $project->settings->default_environment['web_server'];
    $form['server_stack']['web_server']['#type'] = 'value';
    $form['server_stack']['web_server']['#value'] = $web_server_nid;
    $form['server_stack']['web_server_label'] = array(
      '#type' => 'item',
      '#title' => t('Web server'),
      '#markup' => l($web_servers[$web_server_nid], "node/$web_server_nid", array('attributes' => array('target' => '_blank'))),
    );

    $db_servers = hosting_get_servers('db', FALSE);
    $db_server_nid = $project->settings->default_environment['db_server'];
    $form['server_stack']['db_server']['#type'] = 'value';
    $form['server_stack']['db_server']['#value'] = $db_server_nid;
    $form['server_stack']['db_server_label'] = array(
      '#type' => 'item',
      '#title' => t('Database server'),
      '#markup' => l($db_servers[$db_server_nid], "node/$db_server_nid", array('attributes' => array('target' => '_blank'))),
    );
  }

  $form['project_nid'] = array(
    '#value' => $project->nid,
    '#type' => 'value',
  );

  $form['platform_node'] = array(
    '#type' => 'value',
    '#value' => NULL,
  );


  // Remove platform and profile options.
  // @TODO: Patch Aegir so we don't have to load all of these platform options.
  $form['platform']['#access'] = FALSE;
  $form['profile']['#access'] = FALSE;
  $form['git']['#access'] = FALSE;
}

/**
 * Validator for the domain name field: Sets domain according to pattern.
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_projects_create_environment_form_validate_name($element, &$form_state, $form) {

  if (!empty($element['#value'])) {
    $project_node = node_load($form_state['values']['project_nid']);
    $project = $project_node->project;

    // Check existence of the environment name
    if (!empty($project->environments[$element['#value']])) {
      form_set_error('environment_name', t('There is already an environment named %name in this project. Please choose a different name.', array('%name' => $form_state['values']['environment_name'])));
    }

    // Check for illegal chars
    if (!preg_match('!^[a-z0-9_]+$!', $element['#value'])) {
      form_set_error('environment_name', t('The environment name must contain only lowercase letters, numbers, and underscores.'));
    }

    // Generate field prefix and suffix using domain name pattern.
    $form_state['environment_name'] = $element['#value'];
    form_set_value($form['title'], devshop_environment_url($project, $element['#value']), $form_state);

    // Attach a new platform node.
    $platform_node = devshop_prepare_platform_node($project, $element['#value'], $form_state['values']['git_ref'], $form_state['values']['web_server'], $form_state['values']['git']['repo_url']);

    // Set value in form_state.
    form_set_value($form['platform_node'], $platform_node, $form_state);
    form_set_value($form['profile'], 0, $form_state);
    form_set_value($form['platform'], 0, $form_state);

  }
}

/**
 * Helper to prepare the platform object to be attached to the site node.
 *
 * Example:
 *
 *   $platform = devshop_prepare_platform_node($project, $environment_name, $git_ref, $web_server, $prepare_node);
 *   if ($platform = node_submit($platform)) {
 *     node_save($platform);
 *   }
 *
 * @param $project
 *   A fully loaded project object.
 *
 * @param $environment_name
 *   The name of the environment.
 *
 * @param $git_ref
 *   The desired git reference for the environment.
 *
 * @param $web_server
 *   The NID of a web server to use for this platform.
 *
 * @return stdClass
 *   A populated object ready for node_submit and node_save.
 */
function devshop_prepare_platform_node($project, $environment_name, $git_ref, $web_server, $git_remote = NULL) {
  $platform_node = new stdClass();
  $platform_node->type = 'platform';
  $platform_node->title = $project->name . '_' . $environment_name;

  // Specify GIT Url for hosting_platform.module
  if (empty($git_remote)) {
    $platform_node->git_remote = $project->git_url;
  }
  else {
    $platform_node->git_remote = $git_remote;
  }

  $platform_node->git_reference = $git_ref;


  // Enable git pull queue for this platform if chosen.
  if ($project->settings->deploy['method'] == 'queue') {
    // @TODO: Using integers because hosting_git_pull module isn't enabled anymore.
    $platform_node->git['pull_method'] = 1;
  }
  // For devshop "webhook" and "manual" methods, disable git pull for now.
  else {
    $platform_node->git['pull_method'] = 0;
  }


  // Determine which web server to use.
  // If no web server was set and project has a default, make sure we set it.
  if (empty($web_server) && !empty($project->settings->default_environment['web_server'])) {
    $platform_node->web_server = $project->settings->default_environment['web_server'];
  }
  else {
    $platform_node->web_server = $web_server;
  }

  $platform_node->git_root = $project->code_path . '/' . $environment_name;

  // Append drupal_path to repo_path if there is one. If not, repo_path is publish_path.
  if ($project->drupal_path) {
    $platform_node->publish_path = $platform_node->git_root . '/' . $project->drupal_path;
    $platform_node->git_docroot = $project->drupal_path;
  }
  else {
    $platform_node->publish_path = $platform_node->git_root;
  }
  return $platform_node;
}

/**
 * Validator for the "other profile" field.
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_projects_create_environment_form_validate_profile($element, &$form_state, &$form) {
  if ($form_state['values']['install_method']['method'] == 'profile' && $form_state['values']['install_method']['profile'] =='_other' && empty($element['#value'])) {
    form_set_error('install_method][profile_other', t('You must enter a profile name.'));
  }
  elseif ($form_state['values']['install_method']['method'] == 'profile' && $form_state['values']['install_method']['profile'] =='_other') {
    form_set_value($form['install_method']['profile'], $element['#value'], $form_state);
  }
}

/**
 * Validator for the "clone_source" field.
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_projects_create_environment_form_validate_clone_source($element, &$form_state, $form) {
  if ($form_state['values']['install_method']['method'] == 'clone' && $form_state['values']['install_method']['clone_source'] =='_other' && empty($element['#value'])) {
    form_set_error('install_method][clone_source_drush', t('You must enter a drush alias.'));
  }
}

/**
 * Validator for the "Path to SQL" field.
 *
 * @param $element
 * @param $form_state
 * @param $form
 */
function devshop_projects_create_environment_form_validate_import($element, &$form_state, $form) {

  $url_components = parse_url($element['#value']);

  if ($form_state['values']['install_method']['method'] == 'import' && empty($element['#value'])) {
    form_set_error('install_method][import', t('If using the "Import Database" Install Method, you must enter a MySQL connection URL or absolute path to an SQL file.'));
  }
  elseif ($url_components['scheme'] == 'mysql') {
    $database = array(
      'database' => ltrim($url_components['path'], '/'),
      'username' => $url_components['user'],
      'password' => $url_components['pass'],
      'host' => $url_components['host'],
      'driver' => 'mysql', // replace with your database driver
    );
    Database::addConnectionInfo('devshop_remote_db', 'default', $database);

    try {
      db_set_active('devshop_remote_db');
      $tables = db_query('SHOW TABLES');
      drupal_set_message(t('Database connection successful.'));
    }
    catch (\PDOException $e) {
      form_set_error('install_method][import', t('Unable to connect to the database: %e', array('%e' => $e->getMessage())));
    }
    finally {
      db_set_active();
    }
  }
}

/**
 * Validator for site_node_form for node creation, when creating an environment
 * in a project.
 */
function devshop_projects_create_environment_form_submit($form, &$form_state) {

  // @TODO: Clean up install_method values.

}

/**
 * Form element processor for Install Method radio buttons.
 * @param $element
 * @return array
 */
function devshop_environment_method_process($element) {
  $element = form_process_radios($element);

  foreach (element_children($element) as $i) {
    $element[$i]['#label_attributes']['class'] = array(
      'btn btn-link'
    );
    $element[$i]['#wrapper_attributes']['class'] = array(
      'install-method-wrapper'
    );
  }

  return $element;
}

/**
 * Return all available install profiles found in all platforms for this project.
 */
function devshop_environment_add_form_profile_options($project) {


  foreach ($project->environments as $e => $environment) {

    $packages = hosting_package_instances_load(array(
      'package_type' => 'profile',
      'rid' => $environment->platform,
    ));

    foreach ($packages as $instance) {
      $options[$instance->short_name] = $instance->title;
      $options[$instance->short_name] .= ' <small class="text-muted">' . $instance->description . '</small>';
    }
  }
  $options['_other'] = t('Other') . ' <small class="text-muted">' . t('Choose another profile.') . '</small>';
  return $options;
}

/**
 * Helper to output the forms selection for Deploy Hooks.
 *
 * Used in the Projects settings form, environment settings form, deploy task form, and sync form.
 *
 * @param $project
 * @param null $environment
 * @return array
 */
function devshop_environment_deploy_hooks_form($project, $environment = NULL, $task_type = NULL) {

  // If environment not specified, just grab one.
  if (!$environment) {
    $environment = current($project->environments);
    $is_environment_form = FALSE;
  }
  else {
    $is_environment_form = TRUE;
  }
  $return = array(
      '#type' => 'fieldset',
      '#group' => 'project_settings',
      '#title' => t('Deployment Hooks'),
      '#description' => t('Deployment hooks are run whenever your codebase changes. It is recommended to always enable database updates and cache clearing.'),
      '#weight' => -10,
  );

  $return['#project'] = $project;
  $return['#environment'] = $environment;

  // If we are on the project creation wizard, set some sane defaults
  if (current_path() == 'projects/add/settings') {
    $environment_update = FALSE;
    $environment_cache = FALSE;
    $environment_revert = FALSE;
    $environment_dothooks = FALSE;
    $environment_acquia_hooks  = FALSE;
    $environment_composer_install = FALSE;
    $project_update = TRUE;
    $project_cache  = TRUE;
    $project_revert = FALSE;
    $project_dothooks = FALSE;
    $project_acquia_hooks = FALSE;
    $project_composer_install = FALSE;
  }
  else {
    $environment_update = isset($environment->settings->deploy) ? $environment->settings->deploy['update'] : FALSE;
    $environment_cache = isset($environment->settings->deploy) ? $environment->settings->deploy['cache'] : FALSE;
    $environment_revert = isset($environment->settings->deploy) ? $environment->settings->deploy['revert'] : FALSE;
    $environment_dothooks = isset($environment->settings->deploy) ? $environment->settings->deploy['dothooks'] : FALSE;
    $environment_acquia_hooks = isset($environment->settings->deploy) ? $environment->settings->deploy['acquia_hooks'] : FALSE;
    $environment_composer_install = isset($environment->settings->deploy) ? $environment->settings->deploy['composer'] : FALSE;
    $project_update = isset($project->settings->deploy) ? $project->settings->deploy['default_hooks']['update'] : FALSE;
    $project_cache = isset($project->settings->deploy) ? $project->settings->deploy['default_hooks']['cache'] : FALSE;
    $project_revert = isset($project->settings->deploy) ? $project->settings->deploy['default_hooks']['revert'] : FALSE;
    $project_dothooks = isset($project->settings->deploy) ? $project->settings->deploy['default_hooks']['dothooks'] : FALSE;
    $project_acquia_hooks = isset($project->settings->deploy) ? $project->settings->deploy['default_hooks']['acquia_hooks'] : FALSE;
    $project_composer_install = isset($project->settings->deploy) ? $project->settings->deploy['default_hooks']['composer'] : FALSE;
  }

  $return['update'] = array(
      '#type' => 'checkbox',
      '#title' => t('Run database updates.'),
      '#default_value' => empty($environment->settings->deploy) ? $project_update : $environment_update,
  );
  $return['cache'] = array(
      '#type' => 'checkbox',
      '#title' => t('Clear all caches.'),
      '#default_value' => empty($environment->settings->deploy)? $project_cache : $environment_cache,
  );
  $return['revert'] = array(
    '#type' => 'checkbox',
    '#title' => t('Revert all features.'),
    '#description' => t('If features is enabled, revert all of them.'),
    '#default_value' => empty($environment->settings->deploy)? $project_revert : $environment_revert,
  );
  // Look for .hooks or .hooks.yml
  if (!$is_environment_form || isset($project->settings->deploy, $project->settings->deploy['default_hooks']['dothooks'])) {
    $return['dothooks'] = array(
      '#type' => 'checkbox',
      '#title' => t('Run <code>deploy</code> commands in the <code>.hooks</code> file.'),
      '#default_value' => empty($environment->settings->deploy) ? $project_dothooks : $environment_dothooks,
      '#description' => t('You can add your desired deploy hooks to a file in the root folder of your project. This is recommended as it gives your developers control over what happens when their code is deployed. See the !dothooks for more information. <em>If you use a <code>.hooks</code> file you probably want to uncheck the deploy hooks here.</em>', array(
        '!dothooks' => l(t('.hooks documentation'), 'admin/help/devshop_dothooks'),
      )),
    );
  }

  // Look for acquia cloud hooks.
  if (module_exists('devshop_acquia') && (!$is_environment_form || isset($project->settings->deploy, $project->settings->deploy['default_hooks']['acquia_hooks']))) {
    $return['acquia_hooks'] = array(
      '#type' => 'checkbox',
      '#title' => t('Run Acquia Cloud Hooks'),
      '#description' => '<p>' . t('Acquia Cloud Hooks were detected in your project.  Check this box to run all acquia cloud hooks.') . '</p><p>' . t('Available cloud hooks are: "post-code-update" when deploying code via git, "post-code-deploy" when deploying code manually, "post-db-copy" when running a sync task with a database, and "post-files-copy" when running a sync task with files.') . '</p>',
      '#default_value' => empty($environment->settings->deploy) ? $project_acquia_hooks : $environment_acquia_hooks,
    );
  }

  // Allow other modules to alter the deploy hooks form elements.
  // @TODO: Create a new hook: "hook_devshop_deploy_hooks()"
  drupal_alter('devshop_deploy_hooks_form_elements', $return, $is_environment_form);

  // Disable deploy hooks on environment form, deploy task form ,and sync task form.
  if (
    // Environment form
    ($is_environment_form && !$project->settings->deploy['allow_environment_deploy_config']) ||
    ($task_type == 'deploy' && !$project->settings->deploy['allow_environment_deploy_hooks_override'])
  ) {

    foreach (element_children($return) as $i) {
      $return[$i]['#type'] = 'value';
      $return[$i]['#value'] = $project->settings->deploy['default_hooks'][$i];

      $return["{$i}_display"] = array(
        '#type' => 'markup',
        '#markup' => t($return[$i]['#title']),
        '#prefix' => '<div class="list-group-item">' . ($return[$i]['#default_value']? '<i class="fa fa-check-square-o text-success pull-left"></i>': '<i class="fa fa-square-o  text-muted pull-left"></i>'),
        '#suffix' => '</div>'
      );
    }
    $return['#description'] = t('Environment-specific deploy hook configuration is not allowed.');

    $project_node = node_load($project->nid);
    if (node_access('update', $project_node)) {
      $return['#description'] .= t('Visit !link to change.', array(
        '!link' => l(t('Project Settings'), "node/{$project->nid}/edit", array(
          'fragment' => 'edit-project-settings-deploy-default-hooks',
        )),
      ));
    }
  }
  return $return;
}

/**
 * Submit handler for site/environment settings page.
 */
function devshop_projects_environment_settings_submit($form, &$form_state) {

  // Save the environment record.
  $site = $form_state['node'];

  // Prepare record for saving
  // When creating a new environment, there is no environment value.
  if (empty($form_state['values']['environment']['name'])) {
    $environment = new stdClass();
    $environment->project_nid = $form_state['values']['project_nid'];
    $environment->name = $form_state['values']['environment_name'];
    $environment->site = $site->nid;
    $environment->platform = $site->platform;
  }
  else {
    $environment = (object) $form_state['values']['environment'];
  }

  $form_state['values']['environment']['settings']['install_method'] = $form_state['values']['install_method'];

  $environment->settings = serialize($form_state['values']['environment']['settings']);

  // Get platform verify task
  $tasks = hosting_get_tasks('rid', $site->nid);
  $environment->last_task = current($tasks)->nid;

  // Save environment record.
  // If environment already exists, set primary keys.
  $project = node_load($environment->project_nid);
  if (isset($project->project->environments[$environment->name])) {
    $primary_keys = array('project_nid', 'name');
  }
  else {
    $primary_keys = array();
  }



  if (drupal_write_record('hosting_devshop_project_environment', $environment, $primary_keys)) {

    // Remove the "Site dev.drupal.devshop.local has been updated.
    drupal_get_messages('status');

    // Send our own message
    if (!empty($primary_keys)) {
      drupal_set_message(t('Settings saved for environment !env.', array('!env' => l($environment->name, "node/{$environment->site}"))));
    }
    else {
      drupal_set_message(t('Environment !env created in project %project.', array(
        '!env' => l($environment->name, "node/{$environment->site}"),
        '%project' => $project->project_name,
      )));
    }
  }
  else {
    drupal_set_message(t('Environment was not saved for %env in project %project!', array('%env' => $environment->name, '%project' => $project->project_name)));

    watchdog('error', 'Environment record not saved: ' . print_r($environment, 1));
  }

  $form_state['redirect'] = 'node/' . $environment->site;
}


/**
 * Implements hook_form_alter().
 */
function devshop_projects_form_server_node_form_alter(&$form, &$form_state, $form_id) {

  $form['title']['#description'] = t('The hostname of the server. Using a fully-qualified domain name for the server hostname is recommended.');
  $form['title']['#attributes']['placeholder'] = 'cloud1.example.com';

}

/**
 * Implements hook_hosting_site_options_alter().
 */
function devshop_projects_hosting_site_options_alter(&$return, $node) {
  $return['profile'][] = 0;
  $return['platform'][] = 0;
}
