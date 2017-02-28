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
    When I select the radio button "Backups enabled"
    And I select the radio button "Default settings"
    And I press the "Save" button
    Then I should see "created in project"
  
  Scenario: Enable Backup custom setting
    When I select the radio button "Backups enabled"
    And I select the radio button "Site specific settings"
    When I select "1 day" from "Backup interval"
    And I press the "Save" button
    Then I should see "created in project"


  Scenario: Disable Backup setting
    When I select the radio button "Backups disabled"
    And I press the "Save" button
    Then I should see "created in project"
