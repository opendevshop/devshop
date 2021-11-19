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

  $form['projects'] = array(
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

  // Server settings.
  $form['server'] = array(
    '#type' => 'fieldset',
    '#title' => t('DevShop Server Access'),
    '#weight' => -1,
  );
  $form['server']['devshop_public_key'] = array(
    '#title' => t('Public Key'),
    '#description' => theme('item_list', array(
      'items' => array(
        t('The public key for this server. This key will be used when cloning code and syncing data from remote servers.'),
        t('Add this key to your git host to allow this DevShop to clone protected code.'),
        t('Add this key to other hosts to authorize this DevShop to connect and copy data from your existing sites.'),
        t('<em class="text-warning">Warning:</em> If you authorize this key on a production server, any user who can access this DevShop will be able to access that server.')
      ),
    )),
    '#type' => 'textarea',
    '#default_value' => variable_get('devshop_public_key', ''),
  );

  $form['pullhook'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pull on webhook'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['pullhook']['devshop_pull_webhook_ip_acl'] = array(
    '#type' => 'textarea',
    '#title' => t('Control Access by IP'),
    '#default_value' => variable_get('devshop_pull_webhook_ip_acl', DEVSHOP_WEBHOOK_DEFAULT_ALLOWED_IPS),
    '#rows' => 6,
  );

  // Add some help text on what to fill in.
  $items = array();
  $items[] = t('Enter the IP addresses that are allowed to trigger a "Pull Code" task. You may specify address ranges using CIDR notation (e.g. 192.168.1.0/24).');
  $items[] = t("Your local computer's IP address is %ip.", array('%ip' => ip_address()));

  // Show the last IP used to call our webhook. To make adding it easier.
  $last_ip = variable_get('hosting_git_pull_webhook_last_ip', '');
  if ($last_ip) {
    $items[] = t('The last IP to attempt a commit notification was %ip', array('%ip' => $last_ip));
  }
  else {
    $items[] = t('No requests ever detected.  If you add the trigger URL for a project to your git repo host, the IP will be logged and displayed here.');
  }

  $items[] = l(t('View GitHub Webhook IP addresses (listed under "hooks")'), 'https://api.github.com/meta', array(
    'attributes' => array(
        '_target' => 'blank',
    ),
  ));
  $items[] = l(t('View BitBucket Webhook IP addresses'), 'https://ip-ranges.atlassian.com', array(
    'attributes' => array(
        '_target' => 'blank',
    ),
  ));
  $items[] = t('Leave empty to allow access from any IP address.');
  $form['pullhook']['devshop_pull_webhook_ip_acl']['#description'] = theme('item_list', array('items' => $items));

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
