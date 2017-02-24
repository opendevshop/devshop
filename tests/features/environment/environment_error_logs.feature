@api @error_logs @environment
Feature: Change Error Log Settings of the Environment of a project
In order to change Error Log Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    And I click "Environment Settings"
  
  Scenario Outline: Error setting
    Given I <check> "<field>"
    When I press the "Save" button
    Then I should see "created in project"

    Examples:
    |check|field|
    |check|Make error logs available.|
    |check|Make error logs available at|
    |check|Output errors directly to the page. |
    |uncheck|Make error logs available.|
    |uncheck|Make error logs available at|
    |uncheck|Output errors directly to the page. |