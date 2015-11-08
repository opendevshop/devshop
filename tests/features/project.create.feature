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
    Then I fill in "xxxxx" for "Project Code Name"
    And I fill in "http://specialurl" for "Git URL"
    When I press "Next"
    Then I should see "xxxxx"
    And I should see "http://specialurl"

    Then I fill in "live.com" for "Live Domain"
    When I press "Next"
    Then I should see "live.com"

    And I should see "Default Stack"
    And I should see "localhost" in the ".db-server-node" element
    And I should see "devshop.site" in the ".web-server-node" element

    When I press "Cancel"
    Then I should see "Task delete was added to the queue"

