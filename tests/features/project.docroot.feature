@api
Feature: Create a project with Drupal in the docroot.

  In order to start developing a drupal site
  As a project admin
  I need to create a new project

  Scenario: Create a new drupal 8 project with drupal in docroot folder

    When I run drush "en dblog -y"
    Given I am logged in as a user with the "administrator" role
    And I am on the homepage
    When I click "Projects"
    And I click "Start a new Project"
    Then I should see "Step 1"
    Then I fill in "rootproject" for "Project Code Name"
    And I fill in "http://github.com/opendevshop/drupal_docroot" for "Git Repository URL"
    When I press "Next"

    # Step 2
    Then I should see "rootproject"
    And I should not see "The name rootproject is in use. Please try again."
    And I should see "http://github.com/opendevshop/drupal_docroot"

    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    Then print last drush output
    And I reload the page

    When I fill in "docroot" for "Document Root"

    # Step 3
    When I press "Next"
    # Users no longer see this, we wait for verify before showing step 2.
#    Then I should see "Please wait while we connect to your repository and determine any branches."
    And I should see "Document Root"
    And I should see "rootproject"

    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    Then print last drush output
    And I reload the page
    And I reload the page

    Then I should see "Create as many new environments as you would like."
    When I fill in "dev" for "project[environments][NEW][name]"
    And I select "master" from "project[environments][NEW][git_ref]"

    Then I press "Next"
    # Step 4
    And I should see "dev"
    And I should see "master"

    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    Then print last drush output
    And I reload the page

    Then I should see "dev"
    And I should see "master"
    And I reload the page
#    When I click "Process Failed"
    Then I should see "8."
    Then I should not see "Platform verification failed"
    When I select "standard" from "install_profile"
    And I press "Create Project & Environments"

    # FINISH!
    Then print current URL
    When I run drush "wd-show"
    Then print last drush output

    Then I should see "Your project has been created. Your sites are being installed."
    And I should see "Dashboard"
    And I should see "Settings"
    And I should see "Logs"
    And I should see "standard"
#    And I should see "http://github.com/opendevshop/drupal"
    And I should see the link "dev"

    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    Then print last drush output
    Then drush output should not contain "This task is already running, use --force"

    And I reload the page
    Then I should see the link "dev"

#    When I click "Visit Site"
#    Then I should see "Welcome to"
