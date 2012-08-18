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
  
  if ($context->type != 'project'){
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Not a project!'));
  }
 
  // Save our details for each site
  
  
    //  // Save aegir context
    //$platform_hosting_name = "@platform_{$project}_{$env}";
    //$options = array();
    //$options['project'] = d()->name;
    //$options['context_type'] = "platform";
    //$options['root'] = "{$path}/{$env}";
    //$output .= drush_backend_invoke_args('provision-save', array($platform_hosting_name), $options);
    
  //drush_backend_invoke_args('provision-save', array($platform_hosting_name), $options);
    
  
  //print_r($context);
  
  //Resume cron
  // @TODO: how to get hostmaster site name?  @see drush_provision_post_hostmaster_migrate()
  // they use drush_get_option('site_name')
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
  
  return drush_set_error(1, 'STOP!');
}

