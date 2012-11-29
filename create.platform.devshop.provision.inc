<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_devshop_provision_provision_devshop_platform_create() {
 
  $git_url = d()->git_url;
  $branch = d()->branch;
  $project = d()->project_name;
  $path = d()->code_path;

  $args = drush_get_arguments();
  // Validate
  if (empty($git_url)){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Git URL not valid.'));
  }

  if (!file_exists($path)){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Path does not exist!'));
  }

  $platform = drush_get_option('platform-name');
  if (drush_get_option('git-url')) {
    $git_url = drush_get_option('git-url');
  }
  if (drush_get_option('branch')) {
    $bname = drush_get_option('branch');
    $branch = "--branch $bname";
  }
  else {
    $branch = $bname = "";
  }

  if (!$platform) {
    // Try to get it from the command line
    if (count($args) < 2) {
      return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Missing platform name!'));
    }
    else { 
       $platform = $args[1];
    }
  }

  drush_log("Platform name: $platform", 'ok');
  drush_log("Git URL: $git_url", 'ok');
  if ($bname != "") {
    drush_log("Branch: $bname", 'ok');
  }

  // Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');

  // Clone repo
  if (!drush_shell_cd_and_exec($path, 
			       "git clone $branch $git_url $platform")) {
    return drush_set_error('DRUSH_PROVISION_GIT_CLONE_FAILED', dt("Git clone failed in !path.\nThe specific errors are below:\n!errors", array('!path' => $path, '!errors' => implode("\n", drush_shell_exec_output()))));
  }

  // Get a list of branches to see if the branch we wanted was selected
  if (strlen($bname) > 0) {
    if (!drush_shell_cd_and_exec("$path/$platform", "git branch")) { 
      return drush_set_error('DRUSH_PROVISION_GIT_BRANCH_FAILED', dt("Git branch failed in !path.\nThe specific errors are below:\n!errors", array('!path' => "$path/$platform", '!errors' => implode("\n", drush_shell_exec_output()))));
    }
    if (!in_array("* $bname", drush_shell_exec_output())) {
      // Branch not found! Delete local repo directory and abort
      drush_shell_cd_and_exec("$path", "rm -rf $platform");
      return drush_set_error('DRUSH_PROVISION_INVALID_BRANCH', dt("Branch !branch does not exist. Aborting.", array('!branch' => $bname)));
    }
  }

  $platform_hosting_name = "@platform_{$project}_{$platform}";
  $options = array();
  $options['project'] = d()->name;
  $options['context_type'] = "platform";
  $options['root'] = "{$path}/{$platform}";
  $output .= drush_backend_invoke_args('provision-save', array($platform_hosting_name), $options);
    
  // Import platform
  provision_backend_invoke('@hostmaster', 'hosting-import', array($platform_hosting_name));
  
  //Resume cron
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}

