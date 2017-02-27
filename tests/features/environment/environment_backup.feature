@api @backup @environment
Feature: Change Backup Settings of the Environment
In order to change Backup Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    When I click "Environment Settings"
  
  Scenario: Enable Backup default setting
    Given I select "enabled" from "hosting_backup_queue_status"
    When I select "default" from "hosting_backup_queue_settings"
    And I press the "Save" button
    Then I should see "created in project"
  
  Scenario: Enable Backup custom setting
    Given I select "custom" from "hosting_backup_queue_settings"
    When I select "3600" from "hosting_backup_queue_schedule"
    And I press the "Save" button
    Then I should see "created in project"


  Scenario: Disable Backup setting
    Given I select "disabled" from "hosting_backup_queue_status"
    When I press the "Save" button
    Then I should see "created in project"
