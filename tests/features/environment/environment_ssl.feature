@api @ssl @environment
Feature: Change SSL Settings of the Environment of a project
In order to change SSL Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    And I click "Environment Settings"
  
  Scenario: Disable SSL
    Given I select the radio button "Disabled"
    When I press the "Save" button
    Then I should see "created in project"

  Scenario Outline: Enable SSL and Required
    Given I select the radio button "<field>"
    And I select the radio button with the label "Generate a new encryption key"
    And I fill in "New encryption key" with "test"
    When I press the "Save" button
    Then I should see "created in project"

   Examples:
   |field|
   |Enabled|
   |Required|