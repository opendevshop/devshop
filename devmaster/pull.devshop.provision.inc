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
  
  //Get site target
  $target = d()->name;
  $site_path = d()->platform->root? d()->platform->root: d()->root;
  
  //Get options
  $update = drush_get_option('no-update') == FALSE;
  $revert = drush_get_option('no-revert') == FALSE;
  $cache = drush_get_option('no-cache') == FALSE;
  $reset = drush_get_option('no-reset') == FALSE;

  //If reset the first task is git hard reset
  //Execute git reset --hard
  if ($reset) {
    if (drush_shell_cd_and_exec($site_path, 'git reset --hard')) {
      drush_log(dt('Git repository reseted.', array('!path' => $site_path)), 'ok');
      $output = drush_shell_exec_output();
      drush_log(implode("\n", drush_shell_exec_output()), 'ok');
    }
    else {
      return drush_set_error('DRUSH_PROVISION_GIT_PULL_FAILED', dt("Git pull failed in !path.\nThe specific errors are below:\n!errors", array('!path' => $site_path, '!errors' => implode("\n", drush_shell_exec_output()))));
    }
  }

  // Pull latest version of site
  //Execute git pull --rebase
  if (drush_shell_cd_and_exec($site_path, 'git pull --rebase')) {
    drush_log(dt('Git repository pulled.', array('!path' => $site_path)), 'ok');
    $output = drush_shell_exec_output();
    drush_log(implode("\n", drush_shell_exec_output()), 'ok');
  }
  else {
    return drush_set_error('DRUSH_PROVISION_GIT_PULL_FAILED', dt("Git pull failed in !path.\nThe specific errors are below:\n!errors", array('!path' => $site_path, '!errors' => implode("\n", drush_shell_exec_output()))));
  }

  if(count($output) == 1) {
    drush_log('Nothing else needs to be done', 'ok');
    return;
  }
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

