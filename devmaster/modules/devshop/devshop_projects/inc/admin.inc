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

  $form['create'] = array(
    '#group' => 'settings',
    '#type' => 'fieldset',
    '#title' => t('Creating Projects'),
  );
  $form['create']['help'] = array(
    '#title' => t('When creating projects...'),
    '#type' => t('item'),
  );
  $form['create']['options'] = array(
    '#element_validate' => array(
      'devshop_project_settings_ensure_create_method'
    )
  );
  $form['create']['options']['devshop_project_create_custom'] = array(
    '#title' => t('Allow any Git repository'),
    '#description' => t('Allow users to enter any git repository when starting a project.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_project_create_custom', TRUE),
  );

  $form['create']['options']['devshop_project_create_choice'] = array(
    '#title' => t('Allow choice of Git Repositories'),
    '#description' => t('Allow users to choose from a list of git repositories when starting a project.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_project_create_choice', TRUE),
  );
  $form['create']['options']['devshop_project_create_choice_options'] = array(
    '#description' => t('Enter the Git repositories to make available to users, one per line.'),
    '#type' => 'textarea',
    '#default_value' => implode(PHP_EOL, variable_get('devshop_project_create_choice_options', array(
      'git@github.com:opendevshop/drupal-project.git'
    ))),
    '#element_validate' => array('devshop_element_validate_explode_array'),
    '#states' => array(
      'visible' => array(
        ':input[name="devshop_project_create_choice"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['projects']['devshop_project_default_drupal_path'] = array(
    '#title' => t('Default Document Root'),
    '#type' => 'textfield',
    '#description' => t("If index.php isn't in the root of the git repo, you can edit the 'Path to Drupal' setting on each project.  Set a default 'Path to Drupal' here. (For example, an Acquia hosted repo uses 'docroot'.)"),
    '#default_value' => variable_get('devshop_project_default_drupal_path', ''),
  );


  $form['projects'] = array(
    '#type' => 'fieldset',
    '#group' => 'settings',
    '#title' => t('Project Settings'),
  );
  $form['projects']['devshop_project_environment_url_pattern'] = array(
    '#title' => t('Default Environment Domain Name Pattern'),
    '#type' => 'textfield',
    '#description' => t("Each environment will have a system domain name generated for it based on it's name. Use @project for project name, @hostname for '%host', @environment for the environment's name.", array('%host' => $_SERVER['SERVER_NAME'])),
    '#default_value' => variable_get('devshop_project_environment_url_pattern', '@project.@environment.@hostname'),
    '#element_validate' => array(
      'devshop_project_settings_validate_environment_url_pattern',
    ),
  );
  $form['projects']['devshop_project_base_path'] = array(
    '#title' => t('Projects Base Path'),
    '#type' => 'textfield',
    '#description' => t('The default base path that all projects will be created in.  Projects each get their own folder inside this path.'),
    '#default_value' => variable_get('devshop_project_base_path', '/var/aegir/projects'),
  );
  $form['projects']['devshop_projects_allow_custom_code_path'] = array(
    '#prefix' => '<label>' . t('Customizing Projects') . '</label>',
    '#title' => t('Allow projects to have their own code path.'),
    '#description' => t('Allow each project to have a custom "Code Path". If not checked, project paths are set as "/var/aegir/projects/{PROJECT_NAME}.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_custom_code_path', FALSE),
  );
  $form['projects']['devshop_projects_allow_custom_base_url'] = array(
    '#title' => t('Allow projects to have their own environment domain name pattern..'),
    '#description' => t('If enabled each project can set an Environment URL Pattern. Useful if you want all environments to have a custom top-level domain, such as dev.mysite.com.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_custom_base_url', FALSE),
  );
  $form['projects']['hosting_site_allow_custom_file_paths'] = array(
    '#title' => t("Allow sites to customize their paths for uploaded files."),
    '#description' => t("If checked, each site can configure custom paths for the public, private and temporary files."),
    '#type' => 'checkbox',
    '#default_value' => variable_get('hosting_site_allow_custom_file_paths', TRUE),
  );

  $form['projects']['devshop_projects_allow_changing_project_git_url'] = array(
    '#title' => t("Allow changing a project's Git URL."),
    '#description' => t("If checked, a project's git remote URL can be changed. Existing sites will not be changed."),
    '#type' => 'checkbox',
    '#default_value' => variable_get('devshop_projects_allow_changing_project_git_url', TRUE),
  );
//  <div class="help-block">When creating or editing a site, show a field for changing the default files paths. <em>CAUTION:</em> The web server user will be granted write access permission to these folders so that the Drupal site can receive uploaded files. If this is checked, Aegir users can enter any path, and the <code>aegir</code> system user will attempt to make it writable for the web server user, typically <code>www-data</code>, <code>apache</code> or <code>nginx</code>. </div>

  $form['projects']['hosting_require_disable_before_delete'] = array(
    '#prefix' => '<label>' . t('Disabling and Deleting Sites') . '</label>',
    '#title' => t('Sites must be disabled sites before they can be deleted.'),
    '#description' => t("If checked, a site must be disabled before it is deleted. <em>Recommended. If not checked, DevShop users can delete any site.</em>"),
    '#type' => 'checkbox',
    '#default_value' => variable_get('hosting_require_disable_before_delete', TRUE),
  );

//  <label class="control-label" for="edit-hosting-require-disable-before-delete"><input type="checkbox" id="edit-hosting-require-disable-before-delete" name="hosting_require_disable_before_delete" value="1" checked="checked" class="form-checkbox">Require site to be disabled before deletion</label>
//
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
    '#attributes' => array(
      'class' => array('text text-dark')
    ),
    '#items' => array(
      t("DevShop can display it's Public Key for you when creating projects, to make granting access to your git repositories and remote sites easier."),
      t("DevShop clones git repositories as the %user@!server server using this Public Key", $t),
      t("When you log in via SSH as the %user@!server, the <code>ssh</code> command will use this Public Key when connecting to other sites and servers by default. Standard SSH configuration still applies.", $t),
      t("DevShop generates a new unique key pair on installation."),
    ),
  );

  $form['public_key']['devshop_public_key'] = array(
    '#type' => 'item',
    '#title' => t('DevShop Public Key'),
    'key' => array(
      '#type' => 'item',
      '#prefix' => '<pre><code>',
      '#markup' => variable_get('devshop_public_key', t('Unknown public key.')),
      '#suffix' => '</code></pre>',
    ),
  );

  $warnings = [];
  if (empty(variable_get('devshop_public_key', ''))) {
    $warnings[] = t("The 'devshop_public_key' variable is missing. This does <em>not necessarily</em> mean that your keys are missing from the server.");
    $warnings[] = t("If you wish to display the Public Key to users, enter it below, or run the following command on the server to read the existing <code>~/.ssh/id_rsa.pub</code> file automatically.");
  }
  else {
    $form['public_key']['devshop_public_key']['#description'] = t("Note: This key is stored in DevShop as a variable. The web server cannot read it directly. If the SSH keys change on the server, and you wish to show users the correct key when creating projects, you must update the Displayed Public Key below.", $t);
  }

  $form['public_key']['set_key'] = array(
    '#title' => t('Change Displayed Public Key'),
    '#type' => 'fieldset',
    '#attributes' => array(
      'class' => array('panel-warning'),
    ),
    '#collapsible' => true,
    '#collapsed' => true,
    '#weight' => 100,
  );
  $form['public_key']['set_key']['devshop_public_key'] = array(
    '#type' => 'textarea',
    '#title' => t('Displayed Public Key'),
    '#default_value' => variable_get('devshop_public_key', ''),
    '#description' => t('Enter the contents of the <code>~/.ssh/id_rsa.pub</code> file here. To hide the Public Key from users, leave this blank. To set this from the server, run the command: !command', $t),
  );
  $form['#validate'][] = 'devshop_project_settings_validate_displayed_ssh_key';

  if (!devshop_public_key_valid_fingerprint()) {
    $warnings[] = t('Warning: the Displayed SSH Key is not a valid key. Update it below to give users the right key.');
    $form['public_key']['set_key']['#collapsed'] = FALSE;
  }

  $form['public_key']['warnings'] = array(
    '#access' => count($warnings),
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'alert alert-warning text-dark'
      )
    ),
    'warnings' => array(
      '#theme' => 'item_list',
      '#prefix' => '<i class="fa fa-warning pull-left"></i>' ,
      '#items' => $warnings,
    )
  );
  return system_settings_form($form);
}

/**
 * Validate that the displayed SSH key is valid, then store the sanitized version.
 */
function devshop_project_settings_validate_displayed_ssh_key(&$form, &$form_state) {
  if ($fingerprint = devshop_public_key_valid_fingerprint($form_state['values']['devshop_public_key'])) {

  }
  else {
    form_set_error('devshop_public_key', t('SSH Key is not valid. Try again.'));
    $form['public_key']['set_key']['#collapsed'] = FALSE;
  }
}
/**
 * Validate that the default environment domain pattern has @environment and @project.
 */
function devshop_project_settings_validate_environment_url_pattern($element, &$form_state, $form) {
  if (strpos($form_state['values']['devshop_project_environment_url_pattern'], '@environment') === FALSE || strpos($form_state['values']['devshop_project_environment_url_pattern'], '@project') === FALSE) {
    form_error($element, t('The placeholders @project and @environment must be in the Environment Domain Name Pattern.'));
  }
}

/**
 * Validate that the default environment domain pattern has @environment and @project.
 */
function devshop_project_settings_ensure_create_method($element, &$form_state, $form) {
  foreach(element_children($form['create']['options']) as $child) {
    if ($form_state['values'][$child] === 1) {
      return;
    }
  }
  form_error($element, t('You must allow at least one option for creating projects.'));
}