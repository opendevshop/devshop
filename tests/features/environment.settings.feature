Feature: Environment settings save.

  Scenario: Check environment settings.
    Given I am on the homepage
    And I fill in "jon" for "Username"
    And I fill in "jon" for "Password"
    Then I press "Log in"

    When I am at "project/drupal"
    Then I click "Environment Settings"
    When I fill in "Dummy" for "Username"
    And I fill in "Dummy" for "Password"
    Then I press "Save"
    Then I should see "Environment settings saved for dev in project drupal"

    When I click "Environment Settings"
    Then then field "Username" should have the value "Dummy"
