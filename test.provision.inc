<?php

/**
 * Pre provision-devshop-test
 */
function drush_devshop_provision_pre_provision_test(){
}

/**
 * Implements the provision-devshop-test command.
 */
function drush_devshop_provision_provision_test($tests_to_run = '') {
  drush_log(dt('Provision DevShop Run Tests started...'), 'status');

  // Get test info
  $type = drush_get_option('test-type', d()->environment['settings']['testing']['test_type']);

  // Run Simpletest
  if ($type == 'simpletest') {
    drush_log(dt("Running $type tests $tests"), 'ok');
    provision_backend_invoke('@self', 'en simpletest');
    provision_backend_invoke('@self', 'test-run', array($tests_to_run));
  }
  elseif ($type == 'behat') {
    $repo_path = d()->environment['repo_root'];
    $behat_folder_path = drush_get_option('behat-folder-path', d()->environment['settings']['testing']['behat_folder_path']);
    $behat_bin_path = drush_get_option('behat-bin-path', d()->environment['settings']['testing']['behat_bin_path']);

    if (!file_exists($repo_path)){
      return drush_set_error(dt('repo_root does not exist.'), DRUSH_FRAMEWORK_ERROR);
    }
    if (!file_exists($repo_path . $behat_folder_path . '/' . $behat_bin_path)){
      return drush_set_error(dt('File not found at behat_bin_path.'), DRUSH_FRAMEWORK_ERROR);
    }

    $path = $repo_path . $behat_folder_path;
    $cmd = "$behat_bin_path features/$tests_to_run";
    drush_log("Running $cmd in $path");

    if (drush_shell_cd_and_exec($path, $cmd)) {
      $shell_output = drush_shell_exec_output();
      drush_set_context('test_output', $shell_output);
      drush_log(implode("\n", $shell_output), 'ok');
      drush_log(dt('Test Run Passed!'), 'success');
    }
    else {
      $shell_output = drush_shell_exec_output();
      drush_set_context('test_output', $shell_output);
      drush_log(implode("\n", $shell_output), 'error');
      return drush_set_error(dt('Test Run Failed'), DRUSH_FRAMEWORK_ERROR);
    }
  }
}

