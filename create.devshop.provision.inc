<?php

/**
 * Implements the provision-devshop-create command.
 * @TODO: Remove!  This is handled by pre-verify.
 *
 */
function drush_devshop_provision_provision_devshop_create() {
  // Remember, d() is for the PROJECT

/*
  $git_url = d()->git_url;
  $path = d()->code_path;
  $project = d()->project_name;

  $branch = drush_get_option('branch');
  $platform = drush_get_option('platform-name');

  drush_log("PLATFORM: $platform", 'notice');
  drush_log("Branch: $branch", 'notice');

  // Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');

  // Clone repo
  $command = "git clone --branch $branch $git_url $platform";
  if (!drush_shell_cd_and_exec($path, $command)) {
    return drush_set_error('DRUSH_PROVISION_GIT_CLONE_FAILED', dt("Git clone failed in !path.\nThe specific errors are below:\n!errors", array('!path' => $path, '!errors' => implode("\n", drush_shell_exec_output()))));
  }

  drush_log(dt('Git repository cloned:', array('!path' => $path)), 'ok');
  drush_log(implode("\n", drush_shell_exec_output()), 'ok');

  // Save aegir context
  $platform_hosting_name = "@platform_{$project}_{$platform}";
  $options = array();
  $options['project'] = d()->name;
  $options['environment'] = $platform;
  $options['context_type'] = "platform";
  $options['root'] = "{$path}/{$platform}";
  $options['needs_site'] = TRUE;

  $output .= drush_backend_invoke_args('provision-save', array($platform_hosting_name), $options);

  // Import platform
  provision_backend_invoke('@hostmaster', 'hosting-import', array($platform_hosting_name));

  // Resume cron
  provision_backend_invoke('@hostmaster', 'hosting-setup');
  */
}

