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
function drush_provision_provision_devshop_pull() {
  drush_log(dt('Provision DevShop Pull started...'));
  
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  $platform_path = d()->platform->root ? d()->platform->root: d()->root;
  //Get options
  $update = drush_get_option('no-update') == FALSE;
  $revert = drush_get_option('no-revert') == FALSE;
  $cache = drush_get_option('no-cache') == FALSE;
  $sites = drush_get_option('sites');

  // Pull latest version of site
  //Execute git pull --rebase
  if (drush_shell_cd_and_exec($platform_path, 'git pull --rebase')) {
    drush_log(dt('Git repository pulled.', array('!path' => $platform_path)), 'ok');
    $output = drush_shell_exec_output();
    drush_log(implode("\n", drush_shell_exec_output()), 'ok');
  }
  else {
    return drush_set_error('DRUSH_PROVISION_GIT_PULL_FAILED', dt("Git pull failed in !path.\nThe specific errors are below:\n!errors", array('!path' => $platform_path, '!errors' => implode("\n", drush_shell_exec_output()))));
  }

  if (!empty($sites)) {
    foreach ($sites as $key => $target) {
      // Verify the site.
      // @TODO: provision-git-pull should actually handle this. If the files change,
      // it should automatically verify.
      $target = '@' . $target;
       drush_log('hola');
       drush_log($target);
      _drush_sitealias_load_alias($target);
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
   
    }
  } 

  //Resume cron
  // @TODO: how to get hostmaster site name?  @see drush_provision_post_hostmaster_migrate()
  // they use drush_get_option('site_name')
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');

  if(count($output) == 1) {
    drush_log('Nothing else needs to be done', 'ok');
    return;
  }
}

