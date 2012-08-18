<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_create() {
  
  $git_url = d()->git_url;
  $path = d()->code_path;
  $project = d()->project_name;
  
  // Validate
  if (empty($git_url)){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Git URL not valid.'));
  }
  if (file_exists($path)){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Path already exists!'));
  }
  
  // Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  // Change to $path and copy dev to test and live.
  // @TODO: Is better way to do with provision?
  drush_shell_exec("mkdir $path -p");
  // @TODO: Would this be good in provision git?  I feel like its just a wrapper, is it worth it?
  // Maybe it would be better, because of provision_backend_invoke()
  //provision_backend_invoke($target, 'provision-git-clone', array($git_url, $path));
  drush_shell_cd_and_exec($path, "git clone $git_url dev");
  drush_shell_cd_and_exec($path, "cp dev test -rf");
  drush_shell_cd_and_exec($path, "cp dev live -rf");
  
  // @TODO: Create Platforms and Sites, and import into Aegir!
  foreach (array('dev', 'test', 'live') as $env){
    
    // Save aegir context
    $platform_hosting_name = "@platform_{$project}_{$env}";
    $options = array();
    $options['context_type'] = "platform";
    $options['root'] = "{$path}/{$env}";
    $output .= drush_backend_invoke_args('provision-save', array($platform_hosting_name), $options);
    
    // Import platform
    provision_backend_invoke('@hostmaster', 'hosting-import', array($platform_hosting_name));
  }
  
  
  
  //Resume cron
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}

