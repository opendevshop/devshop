@api @domain_name @environment
Feature: Change Domain Name Settings of the Environment
In order to change Domain Name Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    When I click "Environment Settings"
  
  Scenario: Add/Change domain name
    When I fill in "aliases[0]" with "dev2"
    And I press "Add an alias"
    And I press the "Save" button
    Then I should see "created in project"