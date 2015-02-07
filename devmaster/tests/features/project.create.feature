@api
Feature: Create a project
  In order to start developing a drupal site
  As a project admin
  I need to create a new project

  Scenario: Create a new Project
    Given I am logged in as a user with the "administrator" role
    When I click "Projects"
    And I click "Start a new Project"
    Then I should see "Step 1"
    Then I fill in "behatproject" for "Project Code Name"
    And I fill in "http://" for "Git URL"
    When I press "Next"
    Then I should see "Domain Name Settings"
    And I press "Next"

    # @TODO! Selenium is needed for @javascript.
