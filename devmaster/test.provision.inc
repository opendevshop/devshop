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

  if (empty($tests_to_run)) {
    $tests_to_run = d()->environment['settings']['testing']['tests_to_run'];
  }

  // Run Simpletest
  if ($type == 'simpletest') {
    drush_log(dt("Running $type tests $tests_to_run"), 'ok');
    provision_backend_invoke('@self', 'en simpletest');
    provision_backend_invoke('@self', 'test-run', array($tests_to_run));
  }
  elseif ($type == 'behat') {
    // Swap commas for space
    $features_to_run = explode(',', $tests_to_run);
    if (empty($features_to_run)) {
      $features_to_run = array('');
    }

    // Get paths from options or site context.
    $repo_path = d()->environment['repo_root'];
    $behat_folder_path = drush_get_option('behat-folder-path', d()->environment['settings']['testing']['behat_folder_path']);
    $behat_bin_path = drush_get_option('behat-bin-path', d()->environment['settings']['testing']['behat_bin_path']);

    // If no repo at that path, error.
    if (!file_exists($repo_path)){
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('repo_root does not exist.'));
    }
    // If no file at behat bin path, error.
    $yml_path = $repo_path . $behat_folder_path . '/behat.yml';
    if (!file_exists($yml_path)){
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('Please add a file at %file', array('%file' => $yml_path)));
    }

    // Prepare path and command
    $full_behat_folder_path = $repo_path . $behat_folder_path;
    $full_behat_bin_path = $repo_path . $behat_folder_path . '/' . $behat_bin_path;

    // Load the behat.yml from behat_folder_path.
    if (file_exists($full_behat_folder_path . '/behat.yml')) {
      $behat_yml = file_get_contents($full_behat_folder_path . '/behat.yml');
    }
    elseif (file_exists($full_behat_folder_path . '/config/behat.yml')) {
      $behat_yml = file_get_contents($full_behat_folder_path . '/config/behat.yml');
    }
    else {
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('behat.yml file not found in behat_folder_path.'));
    }

    // Run composer update.
    if (drush_shell_cd_and_exec($full_behat_folder_path, 'composer update')) {
      $shell_output = drush_shell_exec_output();
      drush_log(implode("\n", $shell_output), 'status');
    }
    else {
      return drush_set_error('DEVSHOP_COMPOSER_FAIL', dt('composer update failed in ' . $full_behat_folder_path));
    }

    // Write custom behat.yml to temporary folder.
    $environment = d()->environment['name'];
    $url = d()->environment['url'];
    $alias = d()->name;

    $behat_yml .= <<<YML

# DevShop Environment Profile
$environment:
  extensions:
    Behat\MinkExtension:
      base_url: $url
    Drupal\DrupalExtension:
      drush:
        alias: $alias

YML;

    $temporary_behat_yml_file = tempnam($full_behat_folder_path, 'behat.yml.');
    file_put_contents($temporary_behat_yml_file, $behat_yml);

    // Run behat tests for each feature.
    $no_errors = TRUE;
    $test_result = '';
    foreach ($features_to_run as $feature) {

      $cmd = "$full_behat_bin_path features/$feature --config $temporary_behat_yml_file --profile $environment --colors";
      drush_log("Running '$cmd' in $full_behat_folder_path", 'status');

      // If drush_shell_cd_and_exec passes...
      if (drush_shell_cd_and_exec($full_behat_folder_path, $cmd)) {
        $shell_output = drush_shell_exec_output();
        drush_set_context('test_output', $shell_output);
        drush_log(implode("\n", $shell_output), 'ok');
        drush_log(dt('Test Run Passed!'), 'success');
      }
      // If drush_shell_cd_and_exec fails..
      else {
        $shell_output = drush_shell_exec_output();
        drush_set_context('test_output', $shell_output);
        drush_log(implode("\n", $shell_output), 'error');
        $no_errors = drush_set_error('DEVSHOP_TESTS_FAILED', dt('Test Run Failed'));
      }

      // Collect test results.
      $behat_results = implode("\n", $shell_output);
      $time = date('r', time());
      $test_result .= <<<TXT
_____________________________
DevShop Test Run
$time

PATH: $full_behat_folder_path
COMMAND: $cmd
RESULTS:

$behat_results
TXT;
      ;
    }

    // Save drush option for hostmaster to read later.
    drush_set_option('behat_test_results', $test_result);

    // Delete temporary behat config file.
    unlink($temporary_behat_yml_file);

    // If errors were detected, return FALSE;  We've already logged the errors.
    if (!$no_errors) {
      return FALSE;
    }
  }
}

