@api @general @environment
Feature: Change General Settings of the Environment
In order to change General Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    And I click "Environment Settings"
  
  Scenario Outline: Lock database setting
    Given I <check> "<field>"
    When I press the "Save" button
    Then I should see "created in project"

    Examples:
    |check|field|
    |check|Lock Database|
    |uncheck|Lock Database|
    |check|Disable Deploy on Commit|
    |uncheck|Disable Deploy on Commit|

  Scenario: Change client
    Given I fill in "Client" with "admin"
    When I press the "Save" button
    Then I should see "created in project"