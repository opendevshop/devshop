@api
Feature: Create a project
  In order to start developing a drupal site
  As a project admin
  I need to create a new project

  Scenario: Create a new drupal 8 project

    Given users:
      | name       | status | roles          |
      | testadminuser | 1      | administrator |

    Given I am logged in as "testadminuser"
    And I am on the homepage
    When I click "Projects"
    And I click "Start a new Project"
    Then I should see "Step 1"
    Then I fill in "drpl8" for "Project Code Name"
    And I fill in "http://github.com/jonpugh/drupal8" for "Git URL"
    When I press "Next"

    # Step 2
    Then print current URL
#    Then save last response
    Then I should see "drpl8"
    And I should see "http://github.com/jonpugh/drupal8"
#   Uncomment once we have steps to unset the path to drupal.
#    When I fill in "docroot" for "Path to Drupal"

    # Step 3
    When I press "Next"
    Then I should see "Please wait while we connect to your repository and determine any branches."
#    And I should see "Path to Drupal: docroot"

    When I run drush "hosting-tasks -v"
    Then print last drush output
    And I wait "10" seconds
    And I reload the page
    And I reload the page

    Then print last response
    Then I should see "Create as many new environments as you would like."
    When I fill in "dev" for "project[environments][NEW][name]"
    And I select "master" from "project[environments][NEW][git_ref]"

#    And I press "Add environment"
    And I fill in "live" for "project[environments][NEW][name]"
    And I select "master" from "project[environments][NEW][git_ref]"
    And I press "Add environment"
    Then I press "Next"
#    Then print last response

    # Step 4
    And I should see "dev"
    And I should see "live"
    And I should see "master"
    And I should see "master"

    When I run drush "hosting-tasks -v"
    Then print last drush output
    And I wait "10" seconds
    And I reload the page

    Then I should see "dev"
    And I should see "live"
    And I should see "master"

    And I should see "master"
    And I wait "10" seconds
    And I reload the page
#    When I click "Process Failed"
#    Then print last response
    Then I should see "8.0.2"
    Then I should not see "Platform verification failed"
    When I select "standard" from "install_profile"
    And I press "Create Project & Environments"

    # FINISH!
    Then I should see "Your project has been created. Your sites are being installed."
    And I should see "Dashboard"
    And I should see "Settings"
    And I should see "Logs"
    And I should see "standard"
#    And I should see "http://github.com/opendevshop/drupal"
    And I should see the link "dev"
    And I should see the link "live"

    When I run drush "hosting-tasks -v"
    Then print last drush output
    Then drush output should not contain "This task is already running, use --force"

    Then I wait "5" seconds
    And I reload the page
    Then I should see the link "dev"
    Then I should see the link "live"
#    Given I go to "http://dev.drpl8.devshop.travis"
#    When I click "Visit Environment"

# @TODO: Fix our site installation.
#    Then I should see "No front page content has been created yet."

    When I click "Create New Environment"
    And I fill in "test" for "Environment Name"
    And I select "master" from "Branch/Tag"
    Then I press "Create New Environment"
    Then I should see "Environment install in progress."

    When I click "test"
    Then I should see "Environment Dashboard"
    And I should see "Environment Settings"
    When I click "test.drpl8.devshop.travis"
    Then I should see "Welcome to test.drpl8.devshop.travis"
