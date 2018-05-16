# Automated Testing

## Automated Testing with DevShop

DevShop is a testing platform as well as a hosting system.

To get started with automated testing with devshop:

1. Enable the "devshop\_testing" module.

   This can be done in Admin &gt; Hosting, or Admin &gt; Modules, or using drush.

2. Visit your project's settings page.
3. In the "Testing" section, select the type of testing you would like to run.

   Currently the only options are SimpleTest and Behat. This will grow over time.

4. Enter the "Behat folder path" for your project. This is where your Behat composer.json file lives.
5. Hit "Save".

## Setting up your project for testing

1. Create a directory for tests and create a composer.json file in it:

   ```text
   $ mkdir tests
   $ cd tests
   $ vim composer.json
   ```

   Add the following to your `composer.json` file, as instructed in the [Behat Drupal Extension Documentation](https://behat-drupal-extension.readthedocs.org/en/3.1/localinstall.html):

   ```text
   {
    "require": {
      "drupal/drupal-extension": "~3.0"
   },
    "config": {
      "bin-dir": "bin/"
    }
   }
   ```

2. Run `composer install`.
3. Create a `behat.yml` file with the following contents:

   ```text
   default:
    suites:
      default:
        contexts:
          - FeatureContext
          - Drupal\DrupalExtension\Context\DrupalContext
          - Drupal\DrupalExtension\Context\MinkContext
          - Drupal\DrupalExtension\Context\MessageContext
          - Drupal\DrupalExtension\Context\DrushContext
    extensions:
      Behat\MinkExtension:
        goutte: ~
        selenium2: ~
        base_url: http://default
      Drupal\DrupalExtension:
        blackbox: ~
   ```

   **NOTE:** When using devshop for testing, the base\_url and drush alias configuration in your behat.yml file doesn't matter. DevShop will automatically re-write the behat.yml to set the corrent base URL and drush alias. Feel free to set your base\_url to match your localhost or any other domain.

4. Initialize Behat:

   ```text
    $ bin/behat --init
   ```

5. Add the required files to git:

   Since devshop will run `composer install` for you, you only need to include the following files in your behat tests folder to get tests running on devshop:

   ```text
   features
   .gitignore  # Should be set to ignore "vendor"
   behat.yml
   composer.json
   composer.lock
   ```

6. Write your behat tests and put them into the `features` folder. See the [Behat Documentation](http://docs.behat.org/en/v3.0/) for more info on how to write tests.

   A simple test example:

   ```text
   Feature: I can see the homepage
    In order to know what site I am on
    As a visitor
    I should see the site title on the homepage.

    Scenario: See the site title on the homepage.
      Given I am on the homepage
      Then I should see "My Lovely Website"
   ```

7. Deploy your new code to an environment.
8. Check the project settings to ensure the path to tests is accurate. If so, you should see a set of checkboxes with all of your feature files listed.  Leave unselected to run all of the tests.
9. Run Tests:

   You can manually Run Tests by clicking the Environment Settings dropdown \(slider icon\) and then "Run Tests".

   "Run Tests" is also available as a "Deploy Hook", meaning it will trigger a test run after code is deployed \(via git push\).

   This allows for a continuous testing environment.

