<?php

/**
 * Pre provision-git-features-update-commit hook
 */
function drush_provision_pre_provision_devshop_commit(){
  provision_git_is_repo();
}


/**
 * Implements the provision-git-pull command.
 */
function drush_provision_provision_devshop_commit() {

  $site_path = d()->platform->root;
  $target = d()->name;
  
  //Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  // Update All Features
  provision_backend_invoke($target, 'features-update-all');

  //Invoke provision-git-add to add any new files to the index
  provision_backend_invoke($target, 'provision-git-add', 
    array("sites/all/modules/features"), $data);

  //Execute git commit
  //Generate commit message
  $message = array();
  $message[] = "Exported from: $target";
  $message[] = str_repeat('-', strlen($message[0]));
  if (drush_get_option('message')){
    $message[] = drush_get_option('message');
  }
  $message = implode("\n", $message);

  $data = array(
    'message' => $message,
  );
  
  //Invoke provision-git-commit
  provision_backend_invoke($target, 'provision-git-commit', array(), $data);
  
  //Push, if the user wants.
  if (drush_get_option('push')){
    provision_backend_invoke($target, 'provision-git-push');
  }
  
  //Revert, if the user wants.
  if (drush_get_option('revert')){
    provision_backend_invoke($target, 'provision-git-reset');
  }
  
  //Resume cron
  // @TODO: how to get hostmaster site name?  @see drush_provision_post_hostmaster_migrate()
  // they use drush_get_option('site_name')
  // @TODO Better way?  @see drush_hosting_pause()
  provision_backend_invoke('hostmaster', 'hosting-setup');
}
