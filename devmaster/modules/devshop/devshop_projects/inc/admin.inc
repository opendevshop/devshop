<?php

/**
 * @param $element
 */
function devshop_element_validate_explode_array($element, &$form_state, &$form) {
  $lines = explode(PHP_EOL, $element['#value']);
  foreach ($lines as $line) {
    $line = explode('|', $line);
    if (count($line) > 1) {
      $options[$line[0]] = $line[1];
    }
    else {
      $options[] = $line[0];
    }
  }
  form_set_value($element, array_filter($options),$form_state);
}

/**
 * Projects Settings Page
 *
 * All code for admin interface.
 */
function devshop_projects_settings_form($form, &$form_state) {
  $form = array();

  $form['settings'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['projects'] = array(
    '#group' => 'settings',
    '#type' => 'fieldset',
    '#title' => t('Project Templates'),
  );
  $form['projects']['devshop_git_repo_suggestions'] = array(
    '#title' => t('Available Git Repositories'),
    '#description' => t('Enter Git repository URLs, one per line. When creating a new project, these Git repositories are available to clone into new projects.'),
    '#type' => 'textarea',
    '#default_value' => implode(PHP_EOL, variable_get('devshop_git_repo_suggestions', array(
      'git@github.com:opendevshop/devshop-composer-template.git'
    ))),
    '#element_validate' => array('devshop_element_validate_explode_array'),
  );
  $form['projects']['devshop_composer_project_suggestions'] = array(
    '#title' => t('Available Composer Projects'),
    '#description' => t('Enter Composer project names, one per line. These projects are available when creating new projects.'),
    '#type' => 'textarea',
    '#default_value' => implode(PHP_EOL, variable_get('devshop_composer_project_suggestions', array(
      'devshop/composer-template:8.x-dev'
    ))),
    '#element_validate' => array('devshop_element_validate_explode_array'),
  );

  $form['paths'] = array(
    '#type' => 'fieldset',
    '#group' => 'settings',
    '#title' => t('Paths'),
  );
  $form['paths']['devshop_projects_allow_custom_code_path'] = array(
    '#title' => t('Allow custom code path per project.'),
    '#description' => t('Allow each project to have a custom "Code Path". If not checked, project paths are set as "/var/aegir/projects/{PROJECT_NAME}.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_custom_code_path', FALSE),
  );
  $form['paths']['devshop_project_base_path'] = array(
    '#title' => t('Projects Base Path'),
    '#type' => 'textfield',
    '#description' => t('The default base path that all projects will be created in.  Projects each get their own folder inside this path.'),
    '#default_value' => variable_get('devshop_project_base_path', '/var/aegir/projects'),
  );
  $form['paths']['devshop_project_default_drupal_path'] = array(
    '#title' => t('Default Document Root'),
    '#type' => 'textfield',
    '#description' => t("If index.php isn't in the root of the git repo, you can edit the 'Path to Drupal' setting on each project.  Set a default 'Path to Drupal' here. (For example, an Acquia hosted repo uses 'docroot'.)"),
    '#default_value' => variable_get('devshop_project_default_drupal_path', ''),
  );


  $form['urls'] = array(
    '#type' => 'fieldset',
    '#title' => t('URLs'),
    '#group' => 'settings',
  );

  $form['urls']['devshop_projects_allow_changing_project_git_url'] = array(
    '#title' => t("Allow changing a project's Default Git URL."),
    '#description' => t("When editing a project, do not allow changes to the Git URL."),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_changing_project_git_url', TRUE),
  );
  $form['urls']['devshop_projects_allow_custom_base_url'] = array(
    '#title' => t('Allow custom Environment Domain Name Pattern per project.'),
    '#description' => t('If enabled each project can set an Environment URL Pattern.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_custom_base_url', FALSE),
  );
  $form['urls']['devshop_project_environment_url_pattern'] = array(
    '#title' => t('Environment Domain Name Pattern'),
    '#type' => 'textfield',
    '#description' => t("Each environment will have a system domain name generated for it based on it's name. Use @project for project name, @hostname for '%host', @environment for the environment's name.", array('%host' => $_SERVER['SERVER_NAME'])),
    '#default_value' => variable_get('devshop_project_environment_url_pattern', '@project.@environment.@hostname'),
    '#element_validate' => array(
      'devshop_project_settings_validate_environment_url_pattern',
    ),
  );

//  $form['support'] = array(
//      '#type' => 'fieldset',
//      '#title' => t('DevShop Support'),
//  );
//  $form['support']['devshop_support_widget_enable'] = array(
//    '#title' => t('Show Help Widget'),
//    '#description' => t('Uncheck this box if you want to hide the Help widget that appears at the bottom right of the page.'),
//    '#type' => 'checkbox',
//    '#default_value' => variable_get('devshop_support_widget_enable', TRUE),
//  );

  $hostmaster_platform = node_load(hosting_get_hostmaster_platform_nid());
  $hostmaster_server = node_load($hostmaster_platform->web_server);

  $t = array();
  $t['%user'] = variable_get('devshop_app_user', 'aegir');
  $t['!server'] = l($hostmaster_server->title, "node/{$hostmaster_server->nid}");
  $t['!command'] = '<pre>drush @hm vset devshop_public_key "$(cat ~/.ssh/id_rsa.pub)"</pre>';

  // Server settings.
  $form['public_key'] = array(
    '#type' => 'fieldset',
    '#title' => t('DevShop Public Key'),
    '#weight' => -1,
    '#group' => 'settings',
  );
  $form['public_key']['features']  = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'alert alert-info'
      )
    )
  );
  $form['public_key']['features']['list'] = array(
    '#theme' => 'item_list',
    '#prefix' => '<i class="fa fa-question-circle fa-2x pull-left"></i><div>' . t('Grant this DevShop server access to your git repositories and existing websites & servers.'),
    '#suffix' => '</div>',
    '#items' => array(
      t("The Public Key below is shown <em>for your convenience only</em>. It is displayed when creating projects to assist users with connecting their repos."),
      t("It can be used to grant DevShop access to git repository hosts, servers or sites hosted elsewhere."),
      t("DevShop clones git repositories as the %user@!server server using this Public Key", $t),
      t("Actions on the server respect the server's SSH configuration."),
      t("<em class='text-warning'>Warning:</em> If you grant access to a remote server using this key, anyone who can access %user@!server will be able to access the remote.", $t),
      t("If the SSH keys change on the server, you can update this one by running the command: !command", $t)
    ),
    '#attributes' => array(
      'class' => array('text small text-muted')
    )
  );

  $form['public_key']['devshop_public_key'] = array(
    '#type' => 'item',
    '#title' => t('DevShop Public Key'),
    'key' => array(
      '#type' => 'item',
      '#prefix' => '<pre><code>',
      '#markup' => variable_get('devshop_public_key', ''),
      '#suffix' => '</code></pre>',
    ),
  );
  return system_settings_form($form);
}

/**
 * Validate that the default environment domain pattern has @environment and @project.
 */
function devshop_project_settings_validate_environment_url_pattern($element, &$form_state, $form) {
  if (strpos($form_state['values']['devshop_project_environment_url_pattern'], '@environment') === FALSE || strpos($form_state['values']['devshop_project_environment_url_pattern'], '@project') === FALSE) {
    form_error($element, t('The placeholders @project and @environment must be in the Environment Domain Name Pattern.'));
  }
}
