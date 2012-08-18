<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_create($git_url, $path) {
  
  // Validate
  if (empty($git_url)){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Git URL not valid.'));
  }
  if (!file_exists($path)){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Path does not exist.'));
  }
  
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  drush_print("cloning $git_url to $path");
  
  // Clone the git URL to DEV folder
  $dev_path = $path . '/dev';
  //provision_backend_invoke($target, 'provision-git-clone', array($git_url, $path));

  // Change to $path and copy dev to test and live.
  drush_shell_exec("mkdir $path -p");
  drush_shell_cd_and_exec($path, "git clone $git_url dev");
  drush_shell_cd_and_exec($path, "cp dev test -rf");
  drush_shell_cd_and_exec($path, "cp dev live -rf");
  
  //@TODO: Create Platforms and Sites, and import into Aegir!
  
  //Resume cron
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}

