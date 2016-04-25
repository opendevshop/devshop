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
 */
function drush_devshop_provision_provision_test() {

  $project_name = d()->project;
  $project = (object) d("@project_{$project_name}")->project;
  $environment = (object) $project->environments[d()->environment];

  drush_log(dt('Provision DevShop Run Tests started...'), 'status');
  // Get tests to run
  $type = drush_get_option('test-type', $environment->settings['testing']['test_type']);

  if ($environment->settings['testing']['tests_to_run'] === NULL) {
    $tests = array();
  }
  else {
    $tests = array_filter($environment->settings['testing']['tests_to_run']);
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
    $repo_path = $environment->repo_root;
    $behat_folder_path = drush_get_option('behat-folder-path', $project->settings['testing']['behat_folder_path']);

    // If no repo at that path, error.
    if (!file_exists($repo_path)){
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('repo_root does not exist.'));
    }
    // If no file at behat bin path, error.
    $tests_folder = $repo_path  . '/' .$behat_folder_path;
    $yml_path = $repo_path  . '/' .$behat_folder_path . '/behat.yml';
    if (!file_exists($yml_path)){

      $message = <<<TXT
Your project is not yet ready to run Behat tests. Please follow the following steps:

  1. Go to (or create) the '!path' in your repo.  This path is editable in your project settings.
  2. Create a composer.json and behat.yml files according to the instructions at http://behat-drupal-extension.readthedocs.org/en/3.1/localinstall.html

    NOTE: You do not have to edit the "base_url" or "drush alias" properties in behat.yml. DevShop will automatically set those for each environment at runtime.

    However, if you want to run behat test manually from the CLI using bin/behat, you will have to tweak your behat.yml or just use the one that gets generated: behat.ENVIRONMENT_NAME.yml using the --config flag.

  3. Run `bin/behat --init` to create the files needed to run tests. You will see:

    +d features - place your *.feature files here
    +d features/bootstrap - place your context classes here
    +f features/bootstrap/FeatureContext.php - place your definitions, transformations and hooks here

  4. Run `bin/behat` to try to run all your tests.  You should see:

    No scenarios
    No steps
    0m0.00s (9.24Mb)

  5. Create a new test in the "features" folder named "homepage.feature" with the following:

    Feature: Homepage.
      Scenario: Visit homepage.
        Given I am on the homepage
        Then I should see "No front page content has been created yet."

  6. To test the test, edit `behat.yml` and set `base_url` to your site's URL, then run `bin/behat`.
  7. Add the required files and folders to git: composer.json composer.lock behat.yml features
  8. Add a .gitignore file that ignores the `vendor` and `bin` folders.

DevShop will automatically run `composer install` in the test folder before running the tests, so you shouldn't commit the bin/behat file, or the vendor folder.

TXT;


      drush_log(dt($message, array('!path' => $behat_folder_path)), 'error');
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('Your project is not yet ready to run Behat tests. Please follow the instructions and try again.'));
    }

    // Prepare path and command
    $full_behat_folder_path = $repo_path . '/' . $behat_folder_path;
    $full_behat_bin_path = $repo_path . '/' . $behat_folder_path . '/bin/behat';

    // Load the behat.yml from behat_folder_path.
    if (file_exists($full_behat_folder_path . '/behat.yml')) {
      $behat_yml = file_get_contents($full_behat_folder_path . '/behat.yml');
    }
    elseif (file_exists($full_behat_folder_path . '/config/behat.yml')) {
      $behat_yml = file_get_contents($full_behat_folder_path . '/config/behat.yml');
    }
    else {
      return drush_set_error('DEVSHOP_MISSING_FILE', dt('behat.yml file not found in behat_folder_path: ') . $behat_folder_path);
    }

    // Run composer install.
    devshop_drush_process('composer install', $full_behat_folder_path, 'DevShop Testing');

    // Write custom behat.yml to temporary folder.
    $environment_name = $environment->name;
    $url = $environment->url;
    $alias = d()->name;
    $root = d()->root;

    $username = d()->http_basic_auth_username;
    $password = d()->http_basic_auth_password;
    $uri = d()->uri;

    if (!empty($username)) {
      $url = $environment->ssl_enabled?
        "https://$username:$password@$uri":
        "http://$username:$password@$uri";
    }

    $behat_yml .= <<<YML

# DevShop Environment Profile
$environment_name:
  extensions:
    Behat\MinkExtension:
      base_url: $url
    Drupal\DrupalExtension:
      drush:
        alias: $alias
      drupal:
        drupal_root: '$root'
      api_driver: 'drupal'

YML;

    $behat_yml_file = $full_behat_folder_path . "/behat.$environment_name.yml";
    file_put_contents($behat_yml_file, $behat_yml);

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
      if (substr($feature, 0, 1) !== '/') {
        $feature = '/'.$feature;
      }
      $feature_path = empty($feature) ? '' : "features{$feature}";

      // Create Command
      // The extra profile is used so we can dynamically set the URL and drush alias of the behat.yml.
      // the "colors" option is to force it to output ANSI colors. "format-settings" "expand: true" is so
      // Behat will output all of the steps when using "Scenario Outlines".
      $cmd = "$full_behat_bin_path $feature_path --config $behat_yml_file --profile $environment_name --colors --format-settings='{\"expand\": true}'";

      // Run behat command
      devshop_drush_process($cmd, $full_behat_folder_path, 'DevShop Testing');
    }
  }
}

