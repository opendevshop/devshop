<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_platform_create() {
  
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

  if (!$platform) {
    // Try to get it from the command line
    if (count($args) < 2) {
      return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Missing platform name!'));
    }
    else { 
       $platform = $args[1];
    }
  }

  // Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');

  // Clone repo
  drush_shell_cd_and_exec($path, "git clone $git_url $platform");

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

