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
    Then I fill in "projectname" for "Project Code Name"
    And I fill in "http://github.com/opendevshop/drupal" for "Git URL"
    When I press "Next"

    # Step 2
    Then print current URL
#    Then save last response
    Then I should see "projectname"
    And I should see "http://github.com/opendevshop/drupal"
    When I fill in "docroot" for "Path to Drupal"

    # Step 3
    When I press "Next"
    Then I should see "Please wait while we connect to your repository and determine any branches."
    And I should see "Path to Drupal: docroot"

    When I run drush "hosting-tasks -v"
    Then print last drush output
    And I reload the page

#    Then save last response
    Then I should see "Create as many new environments as you would like."
    When I fill in "dev" for "project[environments][NEW][name]"
    And I select "7.x-releases" from "project[environments][NEW][git_ref]"

    And I press "Add environment"
    And I fill in "live" for "project[environments][NEW][name]"
    And I select "7.41" from "project[environments][NEW][git_ref]"
    And I press "Next"

    # Step 4
    Then I should see "Please wait while we clone your repo and verify your drupal code."
    And I should see "dev"
    And I should see "live"
    And I should see "7.41"
    And I should see "7.x-releases"

    When I run drush "hosting-tasks -v"
    Then print last drush output
    And I reload the page

    Then I should see "dev"
    And I should see "test"
    And I should see "7.41"
    And I should see "7.x-releases"
    And I should see "Minimal, Standard"

    # FINISH!
    When I press "Finish"
    Then I should see "Your project has been created. Your sites are being installed."
    And I should see "Dashboard"
    And I should see "Settings"
    And I should see "Logs"
    And I should see "http://github.com/opendevshop/drupal"
    And I should see the link "dev"
    And I should see the link "live"

    When I run drush "hosting-tasks -v"
    Then print last drush output
    And I reload the page

    When I click "Visit Environment"
    Then I should see "No front page content has been created yet."

#    Then I should see "Create as many new environments as you would like."
#
#    When I fill in "dev" for "edit-project-environments-new-name"
#    And I press "Next"
#    Then I should see "Environments saved! Now preparing codebases..."
#    And I should see "Please wait while we clone your repo and verify your drupal code."
#
#    Then I run drush "hosting-tasks"
#    And I reload the page
#    Then I should see "dev"
#    And I should see "master"
#    And I should not see "Platform verification failed"


#  Scenario: Create and Cancel a new Project
#    Given users:
#      | name       | mail       | status | roles          |
#      | admin user | admin@user | 1      | administrator |
#
#    Given I am logged in as "admin user"
#    And I am on the homepage
#    When I click "Projects"
#    And I click "Start a new Project"
#    Then I should see "Step 1"
#    Then I fill in "testproject" for "Project Code Name"
#    And I fill in "http://testurl" for "Git URL"
#    When I press "Next"
#
#    # Project node form
#    Then I should see "testproject"
#    And I should see "http://testurl"

    # @TODO: Fill in all the settings.
#    Then I fill in "docroot" for "Path to Drupal"
#    Then I fill in "/var/aegir/projects/special" for "Code path"
#    Then I fill in "special" for "Base URL"
#    And I select the radio button "Manual Deployment"
#    And I check the box "Allow deploying data from drush aliases"
#    Then I fill in "live.com" for "Live Domain"
#    And I check the box "For new environments, create subdomains under Live Domain."
#    When I press "Next"
#
#    Then I should see "live.com"
#    And I should see "Path to Drupal: docroot"
#
#    And I should see "Default Stack"
#    And I should see "localhost" in the ".db-server-node" element
#    And I should see "devshop.site" in the ".web-server-node" element

    # Go back and Edit
#    When I press "Back"
#    Then the "Path to Drupal" field should contain "docroot"
#    And the "Live Domain" field should contain "live.com"
#    And the "Code path" field should contain "/var/aegir/projects/special"
#    And the "Base URL" field should contain "special"
#    And the "Live Domain" field should contain "live.com"
#    And the "Allow deploying data from drush aliases" checkbox should be checked
#    And the "For new environments, create subdomains under Live Domain." checkbox should be checked
#
#    When I fill in "changedroot" for "Path to Drupal"
#    And I press "Next"
#    Then I should see "Path to Drupal: changedroot"
#
#    When I press "Cancel"
#    Then I should see "Project creation cancelled."
#    And I should be on "projects"
#
#
