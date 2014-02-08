<?php

/**
 * @file install the hostmaster system
 */

 /**
  * Validation hook for devshop-install
  */
function drush_devshop_provision_devshop_install_validate($site = NULL) {
  // set defaults for this whole script
  // those are settings that are not prompted to the user but still overridable
  drush_set_default('version', provision_version());
  $version = drush_get_option('version');
  drush_set_default('profile', 'devshop');
  $profile = drush_get_option('profile');
  $aegir_root = drush_set_default('aegir_root', drush_server_home());
  drush_set_default('root', $aegir_root . '/' . $profile . '-' . $version);
  drush_set_default('r', drush_get_option('root'));
  drush_set_default('script_user', provision_current_user());
  drush_set_default('web_group', _provision_default_web_group());
  drush_set_default('http_service_type', 'apache');
  drush_set_default('drush_make_version', '6.x-2.3');
  drush_set_default('aegir_db_user', 'root');
  drush_set_default('client_name', 'admin');
  $aegir_db_user = drush_get_option('aegir_db_user');
  drush_set_default('makefile', dirname(__FILE__) . '/build-devshop.make');

  drush_print("DEVSHOP automated install script");
  drush_print("==============================================================================");

  if (!$site || !drush_get_option('aegir_host', NULL) || !drush_get_option('aegir_db_pass', NULL) || filter_var(drush_get_option('client_email'), FILTER_VALIDATE_EMAIL)) {
    drush_print("Some settings have not been provided and will now be prompted.
Don't worry: you will get to review those settings after the final install");
  }
  // now we prompt the user for settings if not provided or not sane
  if (!$site) {
    $site = drush_prompt(dt("Aegir frontend URL"), provision_fqdn());
  }
  drush_set_option('site', $site);

  drush_set_default('aegir_host', provision_fqdn());
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
2. Download drush_make
3. Create the Hostmaster frontend platform
4. Install the frontend site
5. Setup the dispatcher (a user cron job)

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
 Aegir DB host: !db_host
 Aegir DB user: !db_user
 Aegir DB password: !db_pass
 Drush make version: !drush_make
 Aegir version: !version
 Aegir platform path: !root
 Aegir makefile: !makefile
 Aegir profile: !profile
 Admin email: !email
 Working Copy: !working
', array('!site' => $site,
         '!fqdn' => drush_get_option('aegir_host'),
         '!home' => drush_get_option('aegir_root'),
         '!user' => drush_get_option('script_user'),
         '!web' => drush_get_option('web_group'),
         '!web_server' => drush_get_option('http_service_type'),
         '!db_host' => drush_get_option('aegir_db_host'),
         '!db_user' => drush_get_option('aegir_db_user'),
         '!db_pass' => is_null(drush_get_option('aegir_db_pass', NULL, 'process')) ? '<previously set>' : '<prompted>',
         '!drush_make' => drush_get_option('drush_make_version'),
         '!version' => drush_get_option('version'),
         '!root' => drush_get_option(array('r', 'root')),
         '!profile' => drush_get_option('profile'),
         '!makefile' => drush_get_option('makefile'),
         '!email' => drush_get_option('client_email'),
         '!working' => drush_get_option('working-copy'),
         )));

  if (!drush_confirm(dt('Do you really want to proceed with the install'))) {
    return drush_set_error('PROVISION_CANCEL_INSTALL', dt('Installation aborted by user'));
  }

  return TRUE;
}

/**
 * Drush command to install hostmaster.
 */
function drush_devshop_provision_devshop_install($site = NULL) {
  $version = drush_get_option('version');
  $site = drush_get_option('site', provision_fqdn());
  $aegir_root = drush_get_option('aegir_root');
  $platform = drush_get_option(array('r', 'root'));

  $aegir_db_user = drush_get_option('aegir_db_user');
  $aegir_db_pass = drush_get_option('aegir_db_pass');

  $server = '@server_master';
  $master_context = array(
    'context_type' => 'server',
    // files
    'remote_host' => drush_get_option('aegir_host'),
    'aegir_root' => $aegir_root,
    'script_user' => drush_get_option('script_user'),
   // apache or nginx or..
    'http_service_type' => drush_get_option('http_service_type'),
    'web_group' => drush_get_option('web_group'),
    'master_url' => "http://" . $site,
  );

  $master_db = sprintf("mysql://%s:%s@%s", urlencode($aegir_db_user), urlencode($aegir_db_pass), drush_get_option('aegir_db_host'));
  if (drush_get_option('aegir_host') == drush_get_option('aegir_db_host')) {
    $master_context['db_service_type'] = 'mysql';
    $master_context['master_db'] = $master_db;
    $dbserver = $server;
  } else {
    $dbserver = '@server_' . drush_get_option('aegir_db_host');
    $dbserver_context = array(
      'remote_host' => drush_get_option('aegir_db_host'),
      'context_type' => 'server',
      'db_service_type' => 'mysql',
      'master_db' => $master_db,
    );
    drush_backend_invoke_args("provision-save", array($dbserver), $dbserver_context);
    provision_backend_invoke($dbserver, 'provision-verify');
  }
  drush_backend_invoke_args("provision-save", array($server), $master_context);
  provision_backend_invoke($server, 'provision-verify');

  // exit if an error has occured.
  if (drush_get_error()) {
    return false;
  }

  if (drush_get_option('backend-only')) {
    return;
  }

  if (!function_exists('drush_make_drush_command')) {
    drush_backend_invoke('dl', array('drush_make-' . drush_get_option('drush_make_version'), 'destination' => $aegir_root . '/.drush/'));
  }

  $platform_name = '@platform_hostmaster';
  drush_backend_invoke_args("provision-save", array($platform_name), array(
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

  $site_name = '@hostmaster';
  drush_backend_invoke_args("provision-save", array($site_name), array(
    'context_type' => 'site',
    'platform' => $platform_name,
    'db_server' => $dbserver,
    'uri' => $site,
    'client_name' => drush_get_option('client_name'),
    'profile' =>  drush_get_option('profile'),
  ));
  $data = provision_backend_invoke($site_name, 'provision-install', array(), array('client_email' => drush_get_option('client_email')));
  provision_backend_invoke($site_name, 'provision-verify');

  // exit if an error has occured.
  if (drush_get_error()) {
    return false;
  }


  drush_print(dt("Initializing the hosting system"));
  provision_backend_invoke($site_name, 'hosting-setup');

  drush_print("");
  drush_print("=====================================");
  drush_print(dt("Aegir DevShop has now been installed."));
  drush_print("=====================================");
  drush_print("");
}
