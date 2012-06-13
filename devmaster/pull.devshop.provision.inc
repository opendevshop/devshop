<?php



/**
 * Pre provision-devshop-deploy
 */
function drush_provision_pre_provision_devshop_pull(){
  provision_git_is_repo();
}


/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_devshop_pull() {
  drush_log(dt('Provision DevShop Pull started...'));
  
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  //Get site target
  $target = d()->name;
  
  //Get options
  $update = drush_get_option('no-update') == FALSE;
  $revert = drush_get_option('no-revert') == FALSE;
  $cache = drush_get_option('no-cache') == FALSE;

  // Pull latest version of site
  provision_backend_invoke($target, 'provision-git-pull');

  // Verify the site.
  // @TODO: provision-git-pull should actually handle this. If the files change,
  // it should automatically verify.
  provision_backend_invoke($target, 'provision-verify');
  
  // update db, unless option is false.
  if ($update){
    provision_backend_invoke($target, 'updb');
  }
  
  // Revert All Features, unless option is false
  if ($revert){
    provision_backend_invoke($target, 'features-revert-all');
  }

  // Clear the whole cache, unless option is false
  if ($cache){
    provision_backend_invoke($target, 'cc all');
  }
  
  //Resume cron
  // @TODO: how to get hostmaster site name?  @see drush_provision_post_hostmaster_migrate()
  // they use drush_get_option('site_name')
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}

