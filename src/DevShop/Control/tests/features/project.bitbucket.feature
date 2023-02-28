@api
Feature: Create a bitbucket project
  In order to start developing a drupal site
  As a project admin
  I need to create a new project

  Scenario: Create a new project from bitbucket

    Given I am logged in as a user with the "administrator" role
    And I am on the homepage
    When I click "Projects"
    And I click "Start a new Project"
    Then I fill in "bitbucketrepo" for "Project Code Name"
    And I select the radio button "Enter a git repository URL" with the id "edit-git-source-custom"
    And I fill in "https://bitbucket.org/jonpugh/drupal.git" for "Git Repository URL"
    When I press "Next"

    # Step 2
    Then I should see "bitbucketrepo"
    And I should see "https://bitbucket.org/jonpugh/drupal.git"
    Then I should see "Please wait while we connect and analyze your repository."
    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    # Then print last drush output
    And I reload the page

    # This repo is at root.
#    Then I fill in "web" for "Document Root"
    And I check the box "Create Pull Request Environments"

    When I press "Next"
    Then I should see "pr2"
    And I should see "justatest created online with Bitbucket"

    Then I should see "Create as many new environments as you would like."
    Then I press "Create Project & Environments"

    # FINISH!
    Then I should see "Your project has been created. Your sites are being installed."
    And I should see the link "justatest created online with Bitbucket"
