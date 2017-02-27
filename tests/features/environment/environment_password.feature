@api @password @environment
Feature: Change Password Settings of the Environment
In order to change Password Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    When I click "Environment Settings"
  
  Scenario: Password Change
    When I fill in "http_basic_auth_username" with "username"
    And I fill in "http_basic_auth_password" with "password"
    And I fill in "http_basic_auth_message" with "Restricted Access"
    And I press the "Save" button
    Then I should see "created in project"