<?php
/**
 * @file devshop.profile
 *
 * DevShop Installation Profile
 */

/**
 * Implements hook_install()
 */
function devmaster_install() {

  // add support for nginx
  if (d()->platform->server->http_service_type === 'nginx') {
    module_enable(array('hosting_nginx'));
  }

  // Bootstrap and create all the initial nodes
  devmaster_bootstrap();

  // Finalize and setup themes, menus, optional modules etc
  devmaster_task_finalize();
}

function devmaster_bootstrap() {
  /* Default node types and default node */
  $types =  node_types_rebuild();

  variable_set('install_profile', 'devmaster');

  // Initialize the hosting defines
  hosting_init();

  /* Default client */
  $node = new stdClass();
  $node->uid = 1;
  $node->type = 'client';
  $node->title = drush_get_option('client_name', 'admin');
  $node->status = 1;
  node_save($node);
  variable_set('hosting_default_client', $node->nid);
  variable_set('hosting_admin_client', $node->nid);

  $client_id = $node->nid;

  /* Default server */
  $node = new stdClass();
  $node->uid = 1;
  $node->type = 'server';
  $node->title = php_uname('n');
  $node->status = 1;
  $node->hosting_name = 'server_master';
  $node->services = array();

  /* Make it compatible with more than apache and nginx */
  $master_server = d()->platform->server;

  // Force https_apache
  hosting_services_add($node, 'http', 'https_apache', array(
    'restart_cmd' => $master_server->http_restart_cmd,
    'port' => 80,
    'https_port' => 443,
    'available' => 1,
  ));

  // Add Certificate service.
  hosting_services_add($node, 'Certificate', 'LetsEncrypt', array(
    'letsencrypt_ca' => 'production'
  ));

  /* examine the db server associated with the hostmaster site */
  $db_server = d()->db_server;
  $master_db = parse_url($db_server->master_db);
  /* if it's not the same server as the master server, create a new node
   * for it */
  if ($db_server->remote_host == $master_server->remote_host) {
    $db_node = $node;
  } else {
    $db_node = new stdClass();
    $db_node->uid = 1;
    $db_node->type = 'server';
    $db_node->title = $master_db['host'];
    $db_node->status = 1;
    $db_node->hosting_name = 'server_' . $db_server->remote_host;
    $db_node->services = array();
  }
  hosting_services_add($db_node, 'db', $db_server->db_service_type, array(
      'db_type' => $master_db['scheme'],
      'db_user' => urldecode($master_db['user']),
      'db_passwd' => isset($master_db['pass']) ? urldecode($master_db['pass']) : '',
      'port' => 3306,
      'available' => 1,
  ));

  drupal_set_message(st('Creating master server node'));
  node_save($node);
  if ($db_server->remote_host != $master_server->remote_host) {
    drupal_set_message(st('Creating db server node'));
    node_save($db_node);
  }
  variable_set('hosting_default_web_server', $node->nid);
  variable_set('hosting_own_web_server', $node->nid);

  variable_set('hosting_default_db_server', $db_node->nid);
  variable_set('hosting_own_db_server', $db_node->nid);

  // Create the hostmaster platform & packages
  $node = new stdClass();
  $node->uid = 1;
  $node->title = 'Drupal';
  $node->type = 'package';
  $node->package_type = 'platform';
  $node->short_name = 'drupal';
  $node->old_short_name = 'drupal';
  $node->description = 'Drupal code-base.';
  $node->status = 1;
  node_save($node);
  $package_id = $node->nid;

  // @TODO: We need to still call these nodes "hostmaster" because the aliases are still @hostmaster and @platform_hostmaster
  $node = new stdClass();
  $node->uid = 1;
  $node->type = 'platform';
  $node->title = 'hostmaster';
  $node->publish_path = d()->root;
  $node->makefile = '';
  $node->verified = 1;
  $node->web_server = variable_get('hosting_default_web_server', 2);
  $node->platform_status = 1;
  $node->status = 1;
  $node->make_working_copy = 0;
  node_save($node);
  $platform_id = $node->nid;
  variable_set('hosting_own_platform', $node->nid);

  $instance = new stdClass();
  $instance->rid = $node->nid;
  $instance->version = VERSION;
  $instance->filename = '';
  $instance->version_code = 1;
  //$instance->schema_version = drupal_get_installed_schema_version('system');
  $instance->schema_version = 0;
  $instance->package_id = $package_id;
  $instance->status = 0;
  $instance->platform = $platform_id;
  hosting_package_instance_save($instance);

  // Create the hostmaster profile package node
  $node = new stdClass();
  $node->uid = 1;
  $node->title = 'devmaster';
  $node->type = 'package';
  $node->old_short_name = 'devmaster';
  $node->description = 'The Devmaster profile.';
  $node->package_type = 'profile';
  $node->short_name = 'devmaster';
  $node->status = 1;
  node_save($node);
  $profile_id = $node->nid;

  $instance = new stdClass();
  $instance->rid = $node->nid;
  $instance->version = VERSION;
  $instance->filename = '';
  $instance->version_code = 1;
  //$instance->schema_version = drupal_get_installed_schema_version('system');
  $instance->schema_version = 0;
  $instance->package_id = $profile_id;
  $instance->status = 0;
  $instance->platform = $platform_id;
  hosting_package_instance_save($instance);

  // Create the main Aegir site node
  $node = new stdClass();
  $node->uid = 1;
  $node->type = 'site';
  $node->title = d()->uri;
  $node->platform = $platform_id;
  $node->client = $client_id;
  $node->db_name = '';
  $node->db_server = $db_node->nid;
  $node->profile = $profile_id;
  $node->import = true;
  $node->hosting_name = 'hostmaster';
  $node->site_status = 1;
  $node->verified = 1;
  $node->status = 1;

  // If this site's hostname has a public DNS record, then LetsEncrypt will
  // work, so set the hostmaster site node https_enabled = HOSTING_HTTPS_REQUIRED
  $records = dns_get_record($node->title);
  foreach ($records as $record) {
    if (
      ($record['type'] == 'A' || $record['type'] == 'CNAME') &&
      $record['ip'] != '127.0.0.1' &&
      $record['ip'] != '127.0.1.1') {
      $node->https_enabled = HOSTING_HTTPS_ENABLED;

      drupal_set_message(t('Public DNS found for !url. Enabling HTTPS with LetsEncrypt.', array(
        '!url' => $node->title,
      )), 'success');
    }
  }
  if ($node->https_enabled != HOSTING_HTTPS_ENABLED) {
    drupal_set_message(t('No public DNS record found for !url. Not enabling LetsEncrypt HTTPS for Hostmaster site.', array(
      '!url' => $node->title,
    )), 'warning');
    $node->https_enabled = HOSTING_HTTPS_DISABLED;
  }

  node_save($node);

  // Save the hostmaster site nid.
  variable_set('aegir_hostmaster_site_nid', $node->nid);

  // Enable the hosting features of modules that we enable by default.
  // The module will already be enabled,
  // this makes sure we also set the default permissions.
  $default_hosting_features = array(
      'hosting_web_server' => 'web_server',
      'hosting_db_server' => 'db_server',
      'hosting_platform' => 'platform',
      'hosting_client' => 'client',
      'hosting_task' => 'task',
      'hosting_server' => 'server',
      'hosting_package' => 'package',
      'hosting_site' => 'site',
      'hosting' => 'hosting',
  );
  hosting_features_enable($default_hosting_features, $rebuild = TRUE, $enable = FALSE);

  // Set the frontpage
  variable_set('site_frontpage', 'devshop');

  // Set the sitename
  variable_set('site_name', 'DevShop');

  // do not allow user registration: the signup form will do that
  variable_set('user_register', 0);

  // This is saved because the config generation script is running via drush, and does not have access to this value
  variable_set('install_url' , $GLOBALS['base_url']);

  // Disable backup queue for sites by default.
  variable_set('hosting_backup_queue_default_enabled', 0);

  // Set hosting_logs default folder.
  variable_set('provision_logs_file_path', '/var/log/aegir');
}

function devmaster_task_finalize() {

  // Set composer_autoloader path to vendor.
  variable_set('composer_autoloader', '../vendor/autoload.php');

  // Enable "boots" theme.
  drupal_set_message(st('Enabling "boots" theme'));
  $theme = 'boots';
  theme_enable(array($theme));
  variable_set('theme_default', $theme);

  // Disable the default Bartik theme
  theme_disable(array('bartik'));

  drupal_set_message(st('Configuring default blocks'));
  devmaster_place_blocks($theme);

  // Save "menu_options" for our content types, so they don't offer to be put in menus.
  variable_set('menu_options_client', '');
  variable_set('menu_options_platform', '');
  variable_set('menu_options_server', '');
  variable_set('menu_options_site', '');

//  drupal_set_message(st('Configuring default blocks'));
//  install_add_block('devshop_hosting', 'devshop_tasks', $theme, 1, 5, 'header', 1);
//
//  // @TODO: CREATE DEVSHOP ROLES!
//  drupal_set_message(st('Configuring roles'));
//  install_remove_permissions(install_get_rid('anonymous user'), array('access content', 'access all views'));
//  install_remove_permissions(install_get_rid('authenticated user'), array('access content', 'access all views'));
//  install_add_permissions(install_get_rid('anonymous user'), array('access disabled sites'));
//  install_add_permissions(install_get_rid('authenticated user'), array('access disabled sites'));
//
//  // Create administrator role
//  $rid = install_add_role('administrator');
//  variable_set('user_admin_role', $rid);

  // Hide errors from the screen.
  variable_set('error_level', 0);

  // Disable Aegir's "Welcome" page
  variable_set('hosting_welcome_page', 0);

  // Disable menu settings for projects
  variable_set('menu_options_project', '');

  // Force things to delete even if things fail.
  variable_set('hosting_delete_force', 1);

  // Don't require users to have a client to create a site.
  variable_set('hosting_client_require_client_to_create_site', 0);

  // Don't automatically import sites.
  variable_set('hosting_platform_automatic_site_import', 0);

  // Make sure "chosen" widget allows "contains" string searching.
  variable_set('chosen_search_contains', 1);
  variable_set('chosen_jquery_selector', 'select:visible');
  variable_set('chosen_minimum_single', 0);
  variable_set('chosen_minimum_multiple', 0);

  // Make sure blocks are setup properly.
//  _block_rehash();

  // Rebuild node access permissions.
  node_access_rebuild();
}

/**
 * Helper function to place block.
 */
function devmaster_place_blocks($theme) {
  $blocks = array(
      array(
          'module' => 'devshop_projects',
          'delta' => 'project_nav',
          'theme' => $theme,
          'status' => 1,
          'weight' => -1,
          'region' => 'header',
          'visibility' => 0,
          'pages' => '',
          'cache' => -1,
      ),
      array(
          'module' => 'devshop_projects',
          'delta' => 'project_create',
          'theme' => $theme,
          'status' => 1,
          'weight' => -1,
          'region' => 'sidebar_first',
          'visibility' => 0,
          'pages' => '',
          'cache' => -1,
      ),
  );

  $query = db_insert('block')->fields(array('module', 'delta', 'theme', 'status', 'weight', 'region', 'visibility', 'pages', 'cache'));

  foreach ($blocks as $block) {
    $query->values($block);
  }
  $query->execute();

}
