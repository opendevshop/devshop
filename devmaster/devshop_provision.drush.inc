<?php
/**
 * @file
 * Drush commands for DevShop Provision
 */

/**
 * Implements hook_drush_init()
 * - Ensures that provision is loaded before devshop_provision
 */
function devshop_provision_drush_init(){
  devshop_provision_register_autoload();
}

/**
 * Register our directory as a place to find provision classes.
 */
function devshop_provision_register_autoload() {
  static $loaded = FALSE;
  if (!$loaded) {
    $loaded = TRUE;
    provision_autoload_register_prefix('Provision_', dirname(__FILE__));
  }
}

/**
 * Implementation of hook_drush_command().
 * Provides provision commands for all devshop tasks.
 */
function devshop_provision_drush_command() {
  $items['provision-devshop-deploy'] = array(
    'description' => 'Deploys a tag or branch to an environment and (optionally) run update.php, clear cache, and revert features.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'update' => 'Run update.php after code pull.',
      'revert' => 'Revert all features after code pull.',
      'cache' => 'Clear all caches after code pull.',
      'reset' => 'Runs "git reset --hard" before pulling.',
      'force' => "Runs always update,revert and cache options, even if files don't change.",
      'test' => 'Queue a test run after the deploy.  (Only works from Hostmaster)',
    ),
    'arguments' => array(
      'git_ref' => 'The git branch or tag to deploy.',
    ),
    'examples' => array(
      'drush @env.project.domain.com provision-devshop-deploy master --cache --update' => 'Triggers a git checkout & pull of branch master to the dev environment, clearing caches and running updates.',
    ),
    'aliases' => array('deploy'),
  );
  $items['provision-devshop-pull'] = array(
    'description' => 'Pull & verify platform code and (optionally) run update.php, clear cache, and revert features.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'update' => 'Run update.php after code pull.',
      'revert' => 'Revert all features after code pull.',
      'cache' => 'Clear all caches after code pull.',
      'reset' => 'Runs "git reset --hard" before pulling.',
      'force' => 'Runs always update,revert and cache options',
    ),
    'arguments' => array(
      'environments' => 'A list of environment names to Pull Code to.',
    ),
    'examples' => array(
      'drush @project_mysite provision-devshop-pull dev test --cache' => 'Triggers a git pull and a clear cache command on @project_mysite\'s dev and test environments.',
    ),
    'aliases' => array('pdp'),
  );
  $items['provision-devshop-commit'] = array(
    'description' => 'Export the site\'s Features and commit the result.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'message' => 'Add a commit message',
      'revert' => 'Force revert all features after exporting and committing.',
    ),
    'arguments' => array(
      'environment' => 'The name of the environment to commit from.',
    ),
    'examples' => array(
      'drush @project_mysite provision-devshop-commit dev --message="changed some settings"' => 'Recreates and Commits all features from the dev environment of @project_mysite with an additional commit message.',
    ),
    'aliases' => array('pdc'),
  );
  /* @TODO: Make devshop dl an addon module.
  $items['provision-devshop-dl'] = array(
    'description' => 'Download modules.  This is just a wrapper for dl, it is here for hostmaster.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'modules' => 'The list of modules to DL.',
    ),
    'aliases' => array('pdl'),
  );
  */
  $items['provision-test'] = array(
    'description' => 'Run a set of tests.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'tests-to-run' => 'The list of tests to run, separated by comma.',
      'test-type' => 'The type of tests to run.  simpletest, behat',
      'behat-folder-path' => 'The path to this sites behat tests.',
      'behat-bin-path' => 'The path to the behat executable within behat-folder-path',
      'output-path' => 'The path to a folder to store results in.  Will be created if it doesn\'t exist',
    ),
    'aliases' => array('test'),
  );
  // @TODO: Remove this once http://drupal.org/node/1995506 is committed
  $items['devshop-install'] = array(
    'description' => dt('Install and verify the DevShop frontend.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'example.com' => dt('The URL of the site to install, optional (default: %host).', array('%host' => provision_fqdn())),
    ),
    'options' => array(
     'http_service_type' => dt('Webserver type to configure (default: %webserver)', array('%webserver' => 'apache')),
     'drush_make_version' => dt('Drush make version to install, if not present (default: %drush_make', array('%drush_make' => '6.x-2.3')),
     'aegir_db_host' => dt('Database host to connect to (default: %host)', array('%host' => 'localhost')),
     'aegir_db_user' => dt('Database user to connect as (default: %user)', array('%user' => 'root')),
     'aegir_db_pass' => dt('Database password to use'),
     'client_email' => dt('Email of the first client to create in the frontend'),
     'client_name' => dt('Name of the first client to create in the frontend (default: %user)', array('%user' => 'admin')),
     'aegir_host' => dt('Fully qualified domain name of the local server (default: %fqdn)', array('%fqdn' => provision_fqdn())),
     'script_user' => dt('User to run the backend as (default: %user)', array('%user' => provision_current_user())),
     'web_group' => dt('Group the webserver is running as (default: %group)', array('%group' => _provision_default_web_group())),
     'version' => dt('The version of this released. (default: %version)', array('%version' => provision_version())),
     'aegir_root' => dt('Install aegir in this home directory (default: %home). Do not change unless you know what you are doing.', array('%home' => drush_server_home())),
     'makefile' => dt('The Drupal makefile to use to build the front end.'),
     'profile' => dt('The Drupal installation profile to use for installing the front-end (default: hostmaster)'),
     'working-copy' => dt('Keep VCS files when building the hostmaster platform using Drush make.')
    ),
  );
  return $items;
}

/**
 * Function for checking if this is a project and we have a repo.
 *
 * Used in pre drush command hooks
 */
function devshop_provision_pre_flight($platform_name = NULL){
  if (d()->type != 'Project'){
    return drush_set_error(DEVSHOP_FRAMEWORK_ERROR, 'All provision-devshop-* commands must be run on a project alias.');
  }
}

/**
 * Append PHP code to Drupal's settings.php file.
 *
 * To use templating, return an include statement for the template.
 *
 * @param $uri
 *   URI for the site.
 * @param $data
 *   Associative array of data from provisionConfig_drupal_settings::data.
 *
 * @return
 *   Lines to add to the site's settings.php file.
 *
 * @see provisionConfig_drupal_settings
 */
function devshop_provision_provision_drupal_config($uri, $data, $config = NULL) {

  $environment = d()->environment['name'];
  $project = d()->environment['project_name'];

  return <<<PHP

  # This site's devshop project name and environment name.
  \$_SERVER['DEVSHOP_ENVIRONMENT'] = '$environment';
  \$_SERVER['DEVSHOP_PROJECT'] = '$project';

  # Include devshop environment configuration settings, if there is any.
  if (file_exists(__DIR__ . '/../all/settings.devshop.php')) {
    include_once(__DIR__ . '/../all/settings.devshop.php');
  }

  if (file_exists(__DIR__ . '/../default/settings.devshop.php')) {
    include_once(__DIR__ . '/../default/settings.devshop.php');
  }

PHP;
}

/**
 * Implements hook_provision_services()
 */
function devshop_provision_provision_services() {
  devshop_provision_register_autoload();
  return array('site_project' => NULL);
}

/**
 * Implements hook_provision_apache_vhost_config()
 *
 * Adds "devshop_project" and "devshop_environment" server variables.
 *
 * @param $uri
 *   URI for the site.
 * @param $data
 *   Associative array of data from Provision_Config_Apache_Site::data.
 *
 * @return
 *   Lines to add to the configuration file.
 *
 * @see Provision_Config_Apache_Site
 * provision_logs_provision_apache_vhost_config
 */
function devshop_provision_provision_apache_vhost_config($uri, $data, Provision_Config_Apache_Site $site_config) {

  $environment = d()->environment['name'];
  $project = d()->environment['project_name'];

  return <<<PHP
  # Saving project and environment server variables
  SetEnv DEVSHOP_PROJECT $project
  SetEnv DEVSHOP_ENVIRONMENT $environment

PHP;
}

/**
 * Implements hook_provision_nginx_vhost_config()
 *
 * Adds "devshop_project" and "devshop_environment" server variables.
 *
 * @param $data
 *   Associative array of data from Provision_Config_Nginx_Server::data.
 *
 * @return
 *   Lines to add to the configuration file.
 *
 * @see Provision_Config_Nginx_Server
 */
function devshop_provision_provision_nginx_vhost_config($data) {

  $environment = d()->environment['name'];
  $project = d()->environment['project_name'];

  return <<<PHP
  # Saving project and environment server variables
  fastcgi_param DEVSHOP_PROJECT $project
  fastcgi_param DEVSHOP_ENVIRONMENT $environment

PHP;
}