<?php
/**
 * @file
 * Drush commands for DevShop Provision
 */

use Symfony\Component\Process\Process;

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

  $environment = d()->hosting_environment;
  $project = d()->hosting_group;
  $site_uri = d()->uri;

  return <<<PHP

  # This site's devshop project name and environment name.
  putenv('DEVSHOP_ENVIRONMENT={$environment}');
  putenv('DEVSHOP_PROJECT={$project}');
  putenv('DEVSHOP_URI={$site_uri}');

  # Include devshop environment configuration settings, if there is any.
  if (file_exists(__DIR__ . '/../all/settings.devshop.php')) {
    include(__DIR__ . '/../all/settings.devshop.php');
  }

  if (file_exists(__DIR__ . '/../default/settings.devshop.php')) {
    include(__DIR__ . '/../default/settings.devshop.php');
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

  $environment = d()->hosting_environment;
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

  $environment = d()->hosting_environment;
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
  if (!empty(d()->project) && empty(d()->hosting_environment)) {
    drush_log('[DEVSHOP] New environment detected.  Rebuilding Registry.', 'ok');
    drush_invoke('registry-rebuild');
  }
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

/**
 * Implementation of drush_hook_provision_pre_COMMAND()
 * for Verify tasks: Writes easier to use drush aliases for each project.
 */
function drush_devshop_provision_provision_install_validate() {

}
