@api @backup @environment
Feature: Change Backup Settings of the Environment
In order to change Backup Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    When I click "Environment Settings"
  
  Scenario Outline: Lock database setting
    Given I select "<option>" from "<field>"
    When I press the "Save" button
    Then I should see "created in project"

    Examples:
    |option|field|
    |enabled|hosting_backup_queue_status|
    |disabled|hosting_backup_queue_status|