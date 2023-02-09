@api
Feature: Create a project and check settings
  In order to start developing a drupal site
  As a project admin
  I need to create a new project

  Scenario: Create a new drupal 8 project

    Given I am logged in as a user with the "administrator" role
    And I am on the homepage
    When I click "Projects"
    And I click "Start a new Project"
    Then I should see "Step 1"
    Then I fill in "composer" for "Project Code Name"
    And I fill in "https://github.com/opendevshop/drupal-project.git" for "Git Repository URL"
    When I press "Next"

    # Step 2
    Then I should see "composer"
    And I should see "https://github.com/opendevshop/drupal-project.git"
    Then I should see "Please wait while we connect and analyze your repository."
    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    # Then print last drush output
    And I reload the page

    Then I fill in "web" for "Document Root"
    And I check the box "Pull Request Environments"

    When I press "Next"
    Then I should see "pr1"
    And I should see "(PR TEST)"

    And I should see "DOCUMENT ROOT web"

    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    And I reload the page
    And I reload the page

    Then I should see "Create as many new environments as you would like."
    When I fill in "dev" for "project[environments][NEW][name]"
    And I select "9.x" from "project[environments][NEW][git_ref]"

#    And I press "Add environment"
#    And I fill in "live" for "project[environments][NEW][name]"
#    And I select "9.x" from "project[environments][NEW][git_ref]"
#    And I press "Add environment"
    Then I press "Create Project & Environments"

    # FINISH!
    Then I should see "Your project has been created. Your sites are being installed."
    And I should see "Dashboard"
    And I should see "Settings"
    And I should see "Logs"

    # @TODO: Fix install profile at hosting_site level: if install_profile is a string, look up package nid before saving.
    # And I should see "standard"
#    And I should see "http://github.com/opendevshop/drupal"
    And I should see the link "dev"
  # @TODO: Fix adding environments.
  #    And I should see the link "live"
    And I should see the link "http://composer.dev.devshop.local.computer"

    And I should see the link "Update readme. (PR TEST)"
    And I should see the link "Add Behat Drupal Extension"

    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    # Then print last drush output
    Then drush output should not contain "This task is already running, use --force"

    And I reload the page
    Then I should see the link "dev"
    Then I should see the link "live"
#    Given I go to "http://dev.composer.devshop.travis"
#    When I click "Visit Environment"

# @TODO: Fix our site installation.
#    Then I should see "No front page content has been created yet."

    When I click "Create New Environment"
    And I fill in "testenv" for "Environment Name"
    And I select "9.x" from "git_ref"

    #@TODO: Check lots of settings

    When I fill in "testuser" for "Username"
    And I fill in "testpassword" for "Password"
    And I fill in "What's the password?" for "Message"
    # @TODO: "Domain Aliases" <label> tag is missing the "for" attribute, so we can't target the string "Domain Aliases"
    And I fill in "test.mysite.com" for "aliases[0]"

    Then I press "Create New Environment"
    Then I should see the link "http://test.mysite.com"
    Then I should see "Environment testenv created in project composer."

    When I run drush "hosting-tasks --force --fork=0 --strict=0"

    Then I should see "Environment Dashboard"
    And I should see "Environment Settings"

    And I click "Environment Settings"
    Then the field "Username" should have the value "testuser"
    Then the field "Password" should have the value "testpassword"
    Then the field "Message" should have the value "What's the password?"

    When I click "Project Settings"
    Then I select "testenv" from "Primary Environment"
    And I press "Save"

    Then I should see "DevShop Project composer has been updated."
    And I should see an ".environment-link .fa-bolt" element

    # When I click "Visit Site"
    Given I am on "http://composer.testenv.devshop.local.computer"
# TODO: Figure out how to test this in travis!
#    Then the response status code should be 401

    Given I am on "http://testuser:testpassword@composer.testenv.devshop.local.computer"
    Then I should see "Welcome!"
    And I should see "Congratulations and welcome to the Drupal community."
    And I should see "composer.testenv.devshop.local.computer"

    Given I am on the homepage
    Then I should see the link "composer"
    And I should see the link "testenv"
    When I click "testenv"
    Then I should not see "Destroy Environment"
    When I click "Disable Environment"
    Then I should see "Are you sure you want to disable composer.testenv.devshop.local.computer?"
    And I press "Disable"
    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    Then I am at "project/composer"
    Then I should see "testenv"
    And I should see "Disabled"

    # @TODO: Test setting for "allow sites to be destroyed"


    # Testing "Manual Install"
    When I click "Create New Environment"
    And I fill in "manualinstall" for "Environment Name"
    And I select "9.x" from "git_ref"
    And I select "manual" from "install_method[method]"
    Then I press "Create New Environment"
    Then I should see "Environment manualinstall created in project composer."

    When I run drush "hosting-tasks --force --fork=0 --strict=0"

    Then I should see "Environment Dashboard"
    And I should see "Manually Installed"
    Given I am on "http://myproject.manual.devshop.local.computer/core/install.php"
    # This fails in CI right now.
    # Then I should see "Choose language"
