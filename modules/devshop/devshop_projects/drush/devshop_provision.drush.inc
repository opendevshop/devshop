<?php
/**
 * @file
 * Drush commands for DevShop Provision
 */

require_once('vendor/autoload.php');

use Symfony\Component\Process\Process;
use Symfony\Component\ClassLoader\ClassLoader;

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
    if (function_exists('provision_autoload_register_prefix')) {
      provision_autoload_register_prefix('Provision_', dirname(__FILE__));
    }
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
      'example.com' => dt('The URL of the site to install, optional (default: %host).', array('%host' => devshop_fqdn())),
    ),
    'options' => array(
     'http_service_type' => dt('Webserver type to configure (default: %webserver)', array('%webserver' => 'apache')),
     'drush_make_version' => dt('Drush make version to install, if not present (default: %drush_make', array('%drush_make' => '6.x-2.3')),
     'aegir_db_host' => dt('Database host to connect to (default: %host)', array('%host' => 'localhost')),
     'aegir_db_user' => dt('Database user to connect as (default: %user)', array('%user' => 'root')),
     'aegir_db_pass' => dt('Database password to use'),
     'client_email' => dt('Email of the first client to create in the frontend'),
     'client_name' => dt('Name of the first client to create in the frontend (default: %user)', array('%user' => 'admin')),
     'aegir_host' => dt('Fully qualified domain name of the local server (default: %fqdn)', array('%fqdn' => devshop_fqdn())),
     'script_user' => dt('User to run the backend as (default: %user)', array('%user' => devshop_current_user())),
     'web_group' => dt('Group the webserver is running as (default: %group)', array('%group' => devshop_default_web_group())),
     'version' => dt('The version of this released. (default: %version)', array('%version' => '1.x')),
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

  $environment = d()->environment;
  $project = d()->project;

  return <<<PHP

  # This site's devshop project name and environment name.
  \$_SERVER['DEVSHOP_ENVIRONMENT'] = '{$environment->name}';
  \$_SERVER['DEVSHOP_PROJECT'] = '{$project->name}';

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
  return array(
    'project' => NULL,
    'Process' => 'Process',
  );
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
function devshop_provision_provision_apache_vhost_config($uri, $data) {

  $environment = d()->environment;
  $project = d()->project;

  if (empty($environment) || empty($project)) {
    return;
  }

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

  $environment = d()->environment;
  $project = d()->project;

  if (empty($environment) || empty($project)) {
    return;
  }

  return <<<PHP
  # Saving project and environment server variables
  fastcgi_param DEVSHOP_PROJECT $project;
  fastcgi_param DEVSHOP_ENVIRONMENT $environment;

PHP;
}

/**
 * Invokes drush_hook_pre_COMMAND()
 *  for hook "devshop_provision" and "updatedb"
 *
 * This function is a hack to fix the problem of moving files around
 * on a pull request environment.
 *
 * When a Pull Request environment is created, the branched code is
 * cloned to the server, and the "live" environment is "cloned".
 *
 * Aegir's "clone" task runs updatedb automatically.  There is no control
 * over this.
 *
 * If the new branch contained moved or missing pieces of the registry, the
 * "updatedb" might fail and cache clears would fail, triggering the rollback
 * process, which deletes the database, deletes the files, and the drush alias!
 *
 * So, you would normally manually run "drush rr" here, but you can't because the
 * site has been destroyed.
 *
 * Hacking into drush's rollback functionality proved challenging, so we're just going
 * to do this:
 *
 *   - Add a "pre updatedb" hook.  The "updatedb" command is called the moment after they import the database.
 *   - In that hook, detect the situation: if the project is set but the environment is
 *     not, it means we have an environment clone in progress.
 *   - Invoke registry rebuild.
 *
 * It Works!
 */
function drush_devshop_provision_pre_updatedb() {

  // When an environment is first cloned, at this point in the process
  // It has a project property, but not the environment.
  // That is what the extra import task is for.
  // We can look for the missing environment
  if (!empty(d()->project) && empty(d()->environment)) {
    drush_log('[DEVSHOP] New environment detected.  Rebuilding Registry.', 'ok');
    drush_invoke('registry-rebuild');
  }
}

/**
 * Run a Symfony Process saving to drush logs.
 */
function devshop_drush_process($command, $cwd = null, $label = 'Process', $env = array(), $log_output = TRUE) {
  drush_log("[$label] $command", 'devshop_command');

  // Merge in env vars, inheriting the CLI's
  if (is_array($env)) {
    $env = array_merge($_SERVER, $env);
  }
  else {
    $env = $_SERVER;
  }

  // Make sure colors always come through
  $env['TERM'] = 'xterm';

  $process = new Process(escapeshellcmd($command), $cwd, $env);
  $process->setTimeout(NULL);
  if ($log_output) {
    $exit_code = $process->run(function ($type, $buffer) {
      if (Process::ERR === $type) {
        drush_log($buffer, 'devshop_info');
      } else {
        drush_log($buffer, 'devshop_info');
      }
    });
  }
  else {
    $exit_code = $process->run();
  }

  // check exit code
  if ($exit_code === 0) {
    drush_log('', 'devshop_ok');
  }
  else {
    drush_log('', 'devshop_error');
    drush_set_error('DEVSHOP_PROCESS_ERROR', dt('Process Failed'));
  }
  return $process->getOutput();
}

/**
 * Find the username of the current running procses
 *
 * This will return the username of the current running user (as seen
 * from posix_geteuid()) and should be used instead of
 * get_current_user() (which looks at the file owner instead).
 *
 * @see get_current_user()
 * @see posix_geteuid()
 *
 * @return
 *   String. The username.
 */
function devshop_current_user() {
  return devshop_posix_username(posix_geteuid());
}

/**
 * Check whether a user is a member of a group.
 *
 * @param user
 *   username or user id of user.
 * @param group
 *   groupname or group id of group.
 *
 * @return
 *   Boolean. True if user does belong to group,
 *   and FALSE if the user does not belong to the group, or either the user or group do not exist.
 */
function devshop_user_in_group($user, $group) {
  // TODO: make these singletons with static variables for caching.
  $user = devshop_posix_username($user);
  $group = devshop_posix_groupname($group);
  if ($user && $group) {
    $info = posix_getgrnam($group);
    if (in_array($user, $info['members'])) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Return the valid system username for $user.
 *
 * @return
 *   Returns the username if found, otherwise returns FALSE
 */
function devshop_posix_username($user) {
  // TODO: make these singletons with static variables for caching.
  // we do this both ways, so that the function returns NULL if no such user was found.
  if (is_numeric($user)) {
    $info = posix_getpwuid($user);
    $user = $info['name'];
  }
  else {
    $info = posix_getpwnam($user);
    $user = $info['name'];
  }
  return $user;
}

/**
 * Return the valid system groupname for $group.
 *
 * @return
 *   Returns the groupname if found, otherwise returns FALSE
 */
function devshop_posix_groupname($group) {
  // TODO: make these singletons with static variables for caching.
  // we do this both ways, so that the function returns NULL if no such user was found.
  if (is_numeric($group)) {
    $info = posix_getgrgid($group);
    $group = $info['name'];
  }
  else {
    $info = posix_getgrnam($group);
    $group = $info['name'];
  }
  return $group;
}

/**
 * Generate a random alphanumeric password.
 *
 * This is a copy of Drupal core's user_password() function. We keep it
 * here in case we need this and don't have a bootstrapped Drupal
 * around.
 *
 * @see user_password()
 */
function devshop_password($length = 10) {
  // This variable contains the list of allowable characters for the
  // password. Note that the number 0 and the letter 'O' have been
  // removed to avoid confusion between the two. The same is true
  // of 'I', 1, and 'l'.
  $allowable_characters = 'abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789';

  // Zero-based count of characters in the allowable list:
  $len = strlen($allowable_characters) - 1;

  // Declare the password as a blank string.
  $pass = '';

  // Loop the number of times specified by $length.
  for ($i = 0; $i < $length; $i++) {

    // Each iteration, pick a random character from the
    // allowable string and append it to the password:
    $pass .= $allowable_characters[mt_rand(0, $len)];
  }

  return $pass;
}

function devshop_default_web_group() {
  $info = posix_getgrgid(posix_getgid());
  $common_groups = array(
    'www-data',
    'apache',
    'nginx',
    'www',
    '_www',
    'webservd',
    'httpd',
    'nogroup',
    'nobody',
    $info['name']);

  foreach ($common_groups as $group) {
    if (devshop_posix_groupname($group)) {
      return $group;
      break;
    }
  }
  return NULL;
}

/**
 * return the FQDN of the machine or provided host
 *
 * this replicates hostname -f, which is not portable
 */
function devshop_fqdn($host = NULL) {
  if (is_null($host)) {
    $host = php_uname('n');
  }
  return strtolower(gethostbyaddr(gethostbyname($host)));
}

//
//if (!function_exists('provision_autoload_register_prefix')) {
//  /**
//   * Register a PECL style prefix with the provision autoloader.
//   *
//   * @param string $prefix
//   *   The class prefix to register.
//   * @param string $dir
//   *   The directory to search for the classes in.
//   * @param bool $prepend
//   *   If the directory should be searched first for the classes in the given
//   *   prefix, set this to TRUE, otherwise, the default, FALSE, is fine.
//   */
//  function provision_autoload_register_prefix($prefix, $dir, $prepend = FALSE) {
//
//    // Get any current directories set for this prefix.
//    $current_prefixes = provision_autoload()->getPrefixes();
//    if (isset($current_prefixes[$prefix])) {
//      $dirs = $current_prefixes[$prefix];
//    }
//    else {
//      $dirs = array();
//    }
//
//    // Now add the new one.
//    if ($prepend) {
//      array_unshift($dirs, $dir);
//    }
//    else {
//      array_push($dirs, $dir);
//    }
//
//    // Set the prefixes.
//    provision_autoload()->addPrefix($prefix, $dirs);
//  }
//}
//
//if (!function_exists('provision_autoload')) {
//  /**
//   * Return an instance of the provision autoloader.
//   *
//   * This will instiatate an instance if it needs to.
//   */
//  function provision_autoload() {
//    static $instance = NULL;
//
//    if (is_null($instance)) {
//      $instance = new ClassLoader();
//      // Activate the autoloader.
//      $instance->register();
//    }
//
//    return $instance;
//  }
//}

/**
 * implementation of provision_verify
 */
function drush_devshop_provision_pre_provision_verify() {
  if (d()->type === 'project') {
    $drush_aliases = new Provision_Config_ProjectAliases(d()->name, d()->project);
    $drush_aliases->write();

    drush_log(dt('Project Drush aliases written.'), 'devshop_log');
  }
}

/**
 * Implementation of drush_hook_post_provision_install
 */
function drush_devshop_provision_post_provision_install() {

  // Sync from the alias.
  if (drush_get_option('site_install_method') == 'clone') {
    drush_log(dt('Environment Clone initiated...'), 'devshop_log');

    // Load Environment and Project.
    $project_name = d()->project;
    $environment_name = d()->environment;
    $environment = d('@project_' . $project_name)->project['environments'][$environment_name];

    // If "clone source" is set for this environment, use provision-sync to load in data.
    if (!empty($environment['settings']['clone_source'])) {
      $source = $environment['settings']['clone_source'];
      $destination = d()->name;
      $command = "drush provision-sync $source $destination --database --files --update-uri --disable-rollback-backup --cache-clear --registry-rebuild";
      devshop_drush_process($command, NULL, 'Cloning site from ' . d($source)->uri);
    }
  }
}
