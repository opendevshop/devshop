<?php

/**
 * Implements the provision-devshop-deploy command.
 */
function drush_provision_provision_devshop_create() {
  
  $git_url = d()->git_url;
  $path = d()->code_path;
  $project = d()->project_name;
  $retry = drush_get_option('retry');

  // Validate
  if (empty($git_url)) {
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Git URL not valid.'));
  }
  if (file_exists($path) && !$retry) {
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Path already exists!'));
  }
  
  // Pause Hostmaster (Mainly for the git pull task)
  // @TODO Better way?  @see drush_hosting_pause()
  exec('crontab -r');
  
  // Change to $path and copy dev to test and live.
  // @TODO: Is better way to do with provision?
  if(drush_shell_exec("mkdir $path -p")) {
    drush_log(dt('Created directory !path.', array('!path' => $path)), 'ok');
  }
  else {
    // Mksie failed. If this is a retry and the directory exist, then it's
    // OK, otherwise PANIC!!
    if (!$retry || !is_dir($path)) {
      drush_log(implode("\n", drush_shell_exec_output()), 'error');
      return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Unable to create directory !path', array('!path' => $path)));
    }
  }

  // @TODO: Would this be good in provision git?  I feel like its just a wrapper, is it worth it?
  // Maybe it would be better, because of provision_backend_invoke()
  //provision_backend_invoke($target, 'provision-git-clone', array($git_url, $path));
  if (drush_shell_cd_and_exec($path, "git clone $git_url dev")) {
    drush_log(dt('Git repository cloned.', array('!path' => $path)), 'ok');
    drush_log(implode("\n", drush_shell_exec_output()), 'ok');
  }
  else {
    drush_log(implode("\n", drush_shell_exec_output()), 'error');
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Git clone failed!'));
  }

  if(drush_shell_cd_and_exec($path, "cp dev test -rf")) {
    drush_log(dt('Repo copied to test.', array()), 'ok');
  }
  else {
    drush_log(implode("\n", drush_shell_exec_output()), 'error');
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Unable to copy repo to test!'));
  }

  if(drush_shell_cd_and_exec($path, "cp dev live -rf")) {
    drush_log(dt('Repo copied to live.', array()), 'ok');
  }
  else {
    drush_log(implode("\n", drush_shell_exec_output()), 'error');
    return drush_set_error('DRUSH_FRAMEWORK_ERROR', dt('Unable to copy repo to live!'));
  }
  
  // @TODO: Create Platforms and Sites, and import into Aegir!
  foreach (array('dev', 'test', 'live') as $env){
    
    // Save aegir context
    $platform_hosting_name = "@platform_{$project}_{$env}";
    $options = array();
    $options['project'] = d()->name;
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

