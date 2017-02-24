@api @deployment_settings @environment
Feature: Change Deplyment Settings of the Environment of a project
In order to change Deployment Settings of the Environment of a project
As an administrator
I need to submit the form
  Background:
    Given I am logged in as a user with the "administrator" role
    And I am at project site
    And I click "Environment Settings"
  
#  Scenario Outline: Error setting
#    Given I <check> "<field>"
#    When I press the "Save" button
#    Then I should see "created in project"

#    Examples:
#    |check|field|
#    |check|Run database updates.|
#    |check|Clear all caches.|
#    |check|Revert all features.|
#    |check|Run composer install.|
#    |check|Run deploy commands in the .hooks file.|
#    |check|Run tests|