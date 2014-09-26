<?php

/**
 * Pre provision-devshop-commit hook
 *
 * Implements drush_hook_pre_COMMAND() hook.
 */
function drush_devshop_provision_pre_provision_devshop_commit(){
  devshop_provision_pre_flight();
}


/**
 * Implements the provision-devshop-commit command.
 */
function drush_devshop_provision_provision_devshop_commit($platform_name = NULL) {

  // Must specify environment to pull
  if (empty($platform_name)){
    return drush_set_error(DRUSH_FRAMEWORK_ERROR, 'You must specify an environment to commit from.');
  }
  
  drush_log(dt('[DEVSHOP] Provision DevShop Commit started...'), 'ok');

  
  // Get platform & site object
  $project_name = d()->project_name;
  $site_alias = '@' . $platform_name . '.' . d()->project['base_url'];
  $platform = d();
  
  $platform_alias = '@' . implode('_', array('platform', $project_name, $platform_name));
  $platform = d($platform_alias);
  
  // Ensure it's a git repo.
  provision_git_is_repo($platform->root);
  
  // Update All Features
  drush_log(dt('[DEVSHOP] Recreating all features...'), 'ok');
  provision_backend_invoke($site_alias, 'features-update-all');

  // Generate commit message.
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

  drush_log(dt('[DEVSHOP] Committing all features...'), 'ok');
 
  // @TODO: Add "path_to_commit option in each project?
  // Invoke provision-git-add to add any new files to the index
  provision_backend_invoke($site_alias, 'provision-git-add', 
    array("sites/all/modules/features"), $data);

  // Invoke provision-git-commit
  provision_backend_invoke($site_alias, 'provision-git-commit', array(), $data);
  
  // Push, unless the user doesn't want to.
  if (drush_get_option('push', TRUE)){
    provision_backend_invoke($site_alias, 'provision-git-push');
  }
  
  // Revert, if the user wants.
  if (drush_get_option('revert')){
    provision_backend_invoke($site_alias, 'provision-git-reset');
  }
}
