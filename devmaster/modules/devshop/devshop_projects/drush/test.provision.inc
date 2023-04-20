<?php

/**
 * Pre provision-devshop-test
 */
function drush_devshop_provision_pre_provision_test(){
  $tests = drush_get_option('tests-to-run');

  if (!empty($tests) && is_string($tests)) {
    drush_set_option('tests-to-run', array_filter(explode(',', $tests)));
  }
  
  if (!empty($tests) && is_array($tests)) {
    drush_set_option('tests-to-run', array_filter($tests));
  }
}

/**
 * Implements the provision-devshop-test command.
 *
 * Do what "verify" task does, so that hook_provision_prepare_environment() gets called
 */
function drush_devshop_provision_provision_test_validate() {
  if (d()->type === 'site') {
    // Do what "verify" task does, so that hook_provision_prepare_environment() gets called
    drush_bootstrap(DRUSH_BOOTSTRAP_DRUPAL_SITE);
    provision_prepare_environment();
  }
}

/**
 * Implements the provision-devshop-test command.
 */
function drush_devshop_provision_provision_test() {

  $project_name = d()->project;
  $project = (object) d("@project_{$project_name}")->project;
  $environment = (object) $project->environments[d()->hosting_environment];

  drush_log(dt('Provision DevShop Run Tests started...'), 'status');

  // Get tests to run
  if (!empty($project->settings['testing']['test_type'])) {
    $type = $project->settings['testing']['test_type'];
  }
  else {
    $type = drush_get_option('test-type', NULL);
  }

  if ($project->settings['testing']['tests_to_run'] === NULL) {
    $tests = array();
  }
  else {
    $tests = array_filter($project->settings['testing']['tests_to_run']);
  }
  $tests_to_run = drush_get_option('tests-to-run', $tests);

  // Run Simpletest
  if ($type == 'simpletest') {
    drush_log(dt("Running $type tests $tests_to_run"), 'ok');
    provision_backend_invoke('@self', 'en simpletest');
    provision_backend_invoke('@self', 'test-run', array($tests_to_run));
  }
  elseif ($type == 'behat') {
    // Get paths from options or site context.
    $repo_path = d()->platform->repo_path? d()->platform->repo_path: d()->platform->root;
    $behat_folder_path = drush_get_option('behat-folder-path', $project->settings['testing']['behat_folder_path']);

    // If no repo at that path, error.
    if (!file_exists($repo_path)){
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('repo_path does not exist.'));
    }
    // If no file at behat bin path, error.
    $tests_folder = $repo_path  . '/' .$behat_folder_path;
    $yml_path = $repo_path  . '/' .$behat_folder_path . '/behat.yml';
    if (!file_exists($yml_path)){

      $message = <<<TXT
Your project is not yet ready to run Behat tests. Please follow the following steps:

  1. Create a folder called '!path' in your repo, or create your own folder 
     and reconfigure your Project Settings > Testing.
  2. Copy the files from https://github.com/opendevshop/devmaster/tree/1.x/modules/devshop/devshop_testing/tests_example
  3. Write more tests!
  
TXT;


      drush_log(dt($message, array('!path' => $tests_folder)), 'error');
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('Your project is not yet ready to run Behat tests. Please follow the instructions and try again.'));
    }

    // Prepare path and command
    $full_behat_folder_path = $repo_path . '/' . $behat_folder_path;
    $full_behat_bin_path = $repo_path . '/' . $behat_folder_path . '/bin/behat';
    $full_behat_yml_path = $full_behat_folder_path . '/behat.yml';

    // Load the behat.yml from behat_folder_path.
    if (file_exists($full_behat_yml_path)) {
      $behat_yml = file_get_contents($full_behat_yml_path);
      $behat_yml_data = \Symfony\Component\Yaml\Yaml::parse($behat_yml);
    }
    elseif (file_exists($full_behat_folder_path . '/config/behat.yml')) {
      $full_behat_yml_path = $full_behat_folder_path . '/config/behat.yml';
      $behat_yml = file_get_contents($full_behat_yml_path);
      $behat_yml_data = \Symfony\Component\Yaml\Yaml::parse($behat_yml);
    }
    else {
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('behat.yml file not found in behat_folder_path: ') . $behat_folder_path);
    }

    // Run composer install.
    provision_process('composer install', $full_behat_folder_path, 'DevShop Testing');

    // Write custom behat.yml to temporary folder.
    $environment_name = $environment->name;
    $alias = d()->name;
    $root = d()->root;

    $username = d()->http_basic_auth_username;
    $password = d()->http_basic_auth_password;
    $uri = d()->uri;
    $url = d()->ssl_enabled?
      "https://$uri":
      "http://$uri";
    
    if (!empty($username)) {
      $url = d()->ssl_enabled?
        "https://$username:$password@$uri":
        "http://$username:$password@$uri";
    }

    $behat_params = array(
      'extensions' => array(
        'Behat\\MinkExtension' => array(
          'base_url' => $url,
        ),
        'Drupal\\DrupalExtension' => array(
          'drupal' => array(
            "drupal_root" => $root,
          ),
          'drush' => array(
            "alias" => $alias,
          )
        ),
      ),
    );

    // Check the behat.yml file for options that will break automation.
    if ($behat_yml_data['default']['extensions']['Behat\MinkExtension']['base_url']) {
      drush_log(dt("Your project's behat.yml file includes the entry default.extensions.Behat\MinkExtension.base_url, preventing devshop from automating this test run. Please remove this entry to continue."), 'error');
    }

    if ($behat_yml_data['default']['extensions']['Drupal\DrupalExtension']['drupal']['drupal_root']) {
      drush_log(dt("Your project's behat.yml file includes the entry default.extensions.Drupal\DrupalExtension.drupal.drupal_root, preventing devshop from automating this test run. Please remove this entry to continue."), 'error');
    }

    if ($behat_yml_data['default']['extensions']['Drupal\DrupalExtension']['drush']['drupal_root']) {
      drush_log(dt("Your project's behat.yml file includes the entry default.extensions.Drupal\DrupalExtension.drupal.drupal_root, preventing devshop from automating this test run. Please remove this entry to continue."), 'error');
    }

    // Run behat tests for each feature.
    $no_errors = TRUE;
    $test_result = '';
    
    // Fill an empty item if empty so we run all the tests.
    if (empty($tests_to_run)) {
      $tests_to_run[] = '';
    }
    
    // Foreach test to run...
    $i = 0;
    foreach ($tests_to_run as $feature) {
      $i++;

      // Check for path.
      if (!empty($feature) && substr($feature, 0, 1) !== '/') {
        $feature = '/'.$feature;
      }
      $feature_path = empty($feature) ? '' : "features{$feature}";

      // Create Command
      // The extra profile is used so we can dynamically set the URL and drush alias of the behat.yml.
      // the "colors" option is to force it to output ANSI colors. "format-settings" "expand: true" is so
      // Behat will output all of the steps when using "Scenario Outlines".
      $cmd = "$full_behat_bin_path $feature_path --colors --strict --format-settings='{\"expand\": true}'";

      // Run behat command
      provision_process($cmd, $full_behat_folder_path, 'DevShop Testing', array(
        'BEHAT_PARAMS' => json_encode($behat_params, JSON_FORCE_OBJECT),
      ), TRUE, 'Test Run Failed');
    }
  }
}

