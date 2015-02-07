@api
Feature: List all projects
  In order to view all of the projects
  As a front-end user
  I need to visit the projects page.

  Scenario: Project list home page
    Given I am logged in as a user with the "authenticated user" role
    When I click "My account"
    Then I should see the heading "History"