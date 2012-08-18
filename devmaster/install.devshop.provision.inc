<?php

/**
 * Pre provision-devshop-deploy
 */
function drush_provision_pre_provision_devshop_install(){

}


/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_install() {
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');  //Check to see that this is a project and we have the necessary info?
  $context = d();
  
  $task = drush_get_context('HOSTING_TASK');

  if ($context->type != 'project'){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Not a project!'));
  }
 
  //Check for install profile. if it doesn't have one, save it.
  print_r($task);
  
  // Save our details for each site
  
  //drush_backend_invoke_args('provision-save', array($platform_hosting_name), $options);
    
  
  //print_r($context);
  
  //Resume cron
  // @TODO: how to get hostmaster site name?  @see drush_provision_post_hostmaster_migrate()
  // they use drush_get_option('site_name')
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}

