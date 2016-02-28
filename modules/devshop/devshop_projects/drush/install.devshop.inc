<?php

/**
 * @file install the hostmaster system
 */

/**
 * Implements drush_COMMAND_pre_validate().
 *
 * Ensure the database cannot be accessed by anonymous users, as it will
 * otherwise fail later in the install, and thus be harder to recover from.
 *
 * @see Provision_Service_db_mysql::grant_host().
 */
function drush_provision_devshop_install_pre_validate() {
  $command = sprintf('mysql -u intntnllyInvalid -h %s -P %s -e "SELECT VERSION()"', drush_get_option('aegir_db_host', 'localhost'), drush_get_option('aegir_db_port', '3306'));
  drush_shell_exec($command);

  if (preg_match("/Access denied for user 'intntnllyInvalid'@'([^']*)'/", implode('', drush_shell_exec_output()), $match)) {
    return;
  }
  elseif (preg_match("/Host '([^']*)' is not allowed to connect to/", implode('', drush_shell_exec_output()), $match)) {
    return;
  }
  else {
    return drush_set_error('PROVISION_DB_CONNECT_FAIL', dt('Dummy connection failed to fail. Either your MySQL permissions are too lax, or the response was not understood. See http://is.gd/Y6i4FO for more information. %msg', array('%msg' => join("\n", drush_shell_exec_output()))));
  }
}

function drush_provision_devshop_install_validate($site = NULL) {
  // set defaults for this whole script
  // those are settings that are not prompted to the user but still overridable
  drush_set_default('version', provision_version());
  drush_set_default('profile', 'devmaster');

  // Get values needed to set other defaults
  $version = drush_get_option('version');
  $aegir_root = drush_set_default('aegir_root', drush_server_home());
  $profile = drush_get_option('profile');

  drush_set_default('root', $aegir_root . '/' . $profile . '-' . $version);
  drush_set_default('r', drush_get_option('root'));
  drush_set_default('script_user', devshop_current_user());
  drush_set_default('web_group', _provision_default_web_group());
  drush_set_default('http_service_type', 'apache');
  drush_set_default('http_port', '80');
  drush_set_default('aegir_db_user', 'root');
  drush_set_default('aegir_db_port', '3306');
  drush_set_default('client_name', 'admin');
  $aegir_db_user = drush_get_option('aegir_db_user');

  // Generate "makefile" message only if there is one set.
  $root = drush_get_option(array('r', 'root'));
  if (is_dir($root) && !drush_get_option('makefile', FALSE)) {
    // Don't assume we know the makefile used to build an existing platform
    $makefile_msg = '';
  }
  else {
    drush_set_default('makefile', dirname(__FILE__) . '/devshop-dev.make');
    $makefile_msg = dt("Aegir makefile: !makefile\n", array('!makefile' => drush_get_option('makefile')));
  }

  // Generate "profile" message only if there is one set.
  if (!drush_get_option('profile', FALSE)) {
    $profile_msg = '';
  }
  else {
    $profile_msg = dt("Aegir install profile: !profile\n", array('!profile' => $profile));
  }

  drush_print("Aegir $version automated install script");
  drush_print("==============================================================================");

  if (!$site || !drush_get_option('aegir_host', NULL) || !drush_get_option('aegir_db_pass', NULL) || filter_var(drush_get_option('client_email'), FILTER_VALIDATE_EMAIL)) {
    drush_print("Some settings have not been provided and will now be prompted.
Don't worry: you will get to review those settings after the final install");
  }
  // now we prompt the user for settings if not provided or not sane
  if (!$site) {
    $site = drush_prompt(dt("Aegir frontend URL"), get_fqdn());
  }
  drush_set_option('site', $site);

  drush_set_default('aegir_host', get_fqdn());
  drush_set_default('aegir_db_host', 'localhost');

  if (is_null(drush_get_option('aegir_db_pass', NULL))) {
    // XXX: may not be portable everywhere?
    system('stty -echo');
    drush_set_option('aegir_db_pass', drush_prompt(dt('MySQL privileged user ("!root") password', array('!root' => $aegir_db_user))));
    system('stty echo');
    print "\n"; // add a newline since the user's didn't print
  }

  if (drush_get_option('aegir_host') == 'localhost') {
    $default_email = 'webmaster@example.com';
  } else {
    $default_email = 'webmaster@' . drush_get_option('aegir_host');
  }
  drush_set_default('client_email', $default_email);
  while (!filter_var(drush_get_option('client_email'), FILTER_VALIDATE_EMAIL) && !drush_get_context('DRUSH_AFFIRMATIVE')) {
    drush_set_option('client_email', drush_prompt(dt("Admin user e-mail"), $default_email));
  }

  drush_print(dt('
This script will operate the following changes in your system:

1. Create server-level configuration directories
2. Create the Hostmaster frontend platform
3. Install the frontend site
4. Setup the dispatcher (a user cron job)

We are making the following assumptions:
 * you have read and are following the install instructions at:
    http://community.aegirproject.org/installing
 * the FQDN of this machine is valid and resolves
 * you are executing this script as your "aegir" user

The following settings will be used:
 Aegir frontend URL: !site
 Master server FQDN: !fqdn
 Aegir root: !home
 Aegir user: !user
 Web group: !web
 Web server: !web_server
 Web server port: !web_server_port
 Aegir DB host: !db_host
 Aegir DB user: !db_user
 Aegir DB password: !db_pass
 Aegir DB port: !db_port
 Aegir version: !version
 Aegir platform path: !root
 Admin email: !email
 !makefile !profile',
    array(
      '!site' => $site,
      '!fqdn' => drush_get_option('aegir_host'),
      '!home' => drush_get_option('aegir_root'),
      '!user' => drush_get_option('script_user'),
      '!web' => drush_get_option('web_group'),
      '!web_server' => drush_get_option('http_service_type'),
      '!web_server_port' => drush_get_option('http_port'),
      '!db_host' => drush_get_option('aegir_db_host'),
      '!db_user' => drush_get_option('aegir_db_user'),
      '!db_pass' => is_null(drush_get_option('aegir_db_pass', NULL, 'process')) ? '<previously set>' : '<prompted>',
      '!db_port' => drush_get_option('aegir_db_port'),
      '!version' => drush_get_option('version'),
      '!root' => $root,
      '!makefile' => $makefile_msg,
      '!profile' => $profile_msg,
      '!email' => drush_get_option('client_email'),
    )));

  if (!drush_confirm(dt('Do you really want to proceed with the install'))) {
    return drush_set_error('PROVISION_CANCEL_INSTALL', dt('Installation aborted'));
  }

  return TRUE;
}

/**
 * Drush command to install hostmaster.
 */
function drush_provision_devshop_install($site = NULL) {
  $version = drush_get_option('version');
  $site = drush_get_option('site', get_fqdn());
  $aegir_root = drush_get_option('aegir_root');
  $platform = drush_get_option(array('r', 'root'));

  $aegir_http_host = drush_get_option('aegir_host');
  $aegir_http_port = drush_get_option('http_port');
  $aegir_db_user = drush_get_option('aegir_db_user');
  $aegir_db_pass = drush_get_option('aegir_db_pass');
  $aegir_db_port = drush_get_option('aegir_db_port');
  $aegir_db_host = drush_get_option('aegir_db_host');

  $server = '@server_master';
  $master_context = array(
    'context_type' => 'server',
    // files
    'remote_host' => $aegir_http_host,
    'aegir_root' => $aegir_root,
    'script_user' => drush_get_option('script_user'),
    // apache or nginx or..
    'http_service_type' => drush_get_option('http_service_type'),
    'http_port' => $aegir_http_port,
    'web_group' => drush_get_option('web_group'),
    'master_url' => "http://" . $site,
    'db_port' => $aegir_db_port,
  );

  $master_db = sprintf("mysql://%s:%s@%s:%s", urlencode($aegir_db_user), urlencode($aegir_db_pass), $aegir_db_host, $aegir_db_port);
  if ($aegir_http_host == $aegir_db_host) {
    $master_context['db_service_type'] = 'mysql';
    $master_context['master_db'] = $master_db;
    $dbserver = $server;
  } else {
    $dbserver = '@server_' . $aegir_db_host;
    $dbserver_context = array(
      'remote_host' => $aegir_db_host,
      'context_type' => 'server',
      'db_service_type' => 'mysql',
      'master_db' => $master_db,
      'db_port' => $aegir_db_port,
    );
    drush_invoke_process('@none', "provision-save", array($dbserver), $dbserver_context);
    provision_backend_invoke($dbserver, 'provision-verify');
  }
  drush_invoke_process('@none', "provision-save", array($server), $master_context);
  provision_backend_invoke($server, 'provision-verify');

  // exit if an error has occured.
  if (drush_get_error()) {
    return false;
  }

  if (drush_get_option('backend-only')) {
    return;
  }

  $platform_name = '@platform_hostmaster';
  drush_invoke_process('@none', "provision-save", array($platform_name), array(
    'context_type' => 'platform',
    'server' => $server,
    'web_server' => $server,
    'root' => $platform,
    'makefile' => drush_get_option('makefile'),
  ));
  // propagate working-copy args downward
  $options = array();
  if (drush_get_option('working-copy')) {
    $options['working-copy'] = 1;
  }
  provision_backend_invoke($platform_name, 'provision-verify', array(), $options);

  // exit if an error has occured.
  if (drush_get_error()) {
    return false;
  }

  drush_set_default('profile', 'devmaster');
  $profile = drush_get_option('profile');

  $site_name = '@hostmaster';
  drush_invoke_process('@none', "provision-save", array($site_name), array(
    'context_type' => 'site',
    'platform' => $platform_name,
    'db_server' => $dbserver,
    'uri' => $site,
    'client_name' => drush_get_option('client_name'),
    'profile' => $profile,
  ));
  $data = provision_backend_invoke($site_name, 'provision-install', array(), array('client_email' => drush_get_option('client_email')));
  provision_backend_invoke($site_name, 'provision-verify');

  // exit if an error has occured.
  if (drush_get_error()) {
    return false;
  }


  drush_print(dt("Initializing the hosting system"));
  drush_invoke_process('@none', 'cache-clear', array('drush'));
  provision_backend_invoke($site_name, 'hosting-setup');

  drush_print("");
  drush_print("==============================================================================");
  drush_print("");
  drush_print("");
  drush_print(dt("Congratulations, Aegir has now been installed."));
  drush_print("");
  drush_print(dt("You should now log in to the Aegir frontend by opening the following link in your web browser:"));
  drush_print("");
  drush_print($data['context']['login_link']);
  drush_print("");
  drush_print("");
  drush_print("==============================================================================");
  drush_print("");
}

/**
 *  Implements drush_hook_post_hostmaster_install().
 */
function drush_provision_post_devshop_install() {
  $backend_only = drush_get_option('backend-only');
  if (empty($backend_only)) {
    drush_invoke_process('@hostmaster', 'cache-clear', array('drush'));
  }
}
