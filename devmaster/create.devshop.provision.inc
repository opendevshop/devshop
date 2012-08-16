<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_create($git_url, $path) {
  drush_log(dt('Provision DevShop Create started...'));
  
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

