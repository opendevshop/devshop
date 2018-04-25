#@api
#Feature: Pull Request Environments
#  In order to review a pull request
#  As a project admin
#  I need to have a copy of the site running on the code from the PR
#
#  @api
#  Scenario: Create a new drupal 8 project
#
#    Given I am logged in as a user with the "administrator" role
#    Given I am at "project/drupal"
#    When I submit a pull-request
#
#    And I am on the homepage
#    When I click "Projects"
#    And I click "Start a new Project"
#    Then I should see "Step 1"
#    Then I fill in "prproject" for "Project Code Name"
#    And I fill in "http://github.com/opendevshop/drupal_docroot.git" for "Git URL"
#    When I press "Next"
#
#    # Step 2
#    Then I should see "prproject"
#    And I should see "http://github.com/opendevshop/drupal_docroot.git"
#    When I fill in "docroot" for "Path to Drupal"
#    And I check the box "Create Environments for Pull Requests"
#    And I check the box "Delete Pull Request Environments"
#    And I check the box "Reinstall Pull Request Environments on every git push."
#
#    # Step 3
#    When I press "Next"
#    Then I should see "Please wait while we connect to your repository and determine any branches."
##    And I should see "Path to Drupal: docroot"
#
#    When I run drush "hosting-tasks --force --fork=0 --strict=0"
#    Then print last drush output
#    And I reload the page
#    And I reload the page
#
#    Then I should see "Create as many new environments as you would like."
#    When I fill in "mirror" for "project[environments][NEW][name]"
#    And I select "master" from "project[environments][NEW][git_ref]"
#    Then I press "Next"
#
#    # Step 4
#    When I run drush "hosting-tasks --force --fork=0 --strict=0"
#    Then print last drush output
#    And I reload the page
#
#    And I reload the page
##    When I click "Process Failed"
#    Then I should see "8."
#    Then I should not see "Platform verification failed"
#    When I select "standard" from "install_profile"
#
##    Then I break
#
#    And I press "Create Project & Environments"
#
#    # FINISH!
#    Then I should see "Your project has been created. Your sites are being installed."
#
#    When I run drush "hosting-tasks --force --fork=0 --strict=0"
#    Then print last drush output
#    Then drush output should not contain "This task is already running, use --force"
#
#   Then I click "Project Settings"
#   And I select "mirror" from "Pull Request Environment Creation Method"
#
#   Then I press "Save"
#   Then I should see "DevShop Project drupal has been updated."
#
#
#
#   # @TODO: Deliver demo PR payloads
