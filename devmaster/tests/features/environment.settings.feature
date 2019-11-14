#Feature: Create an environment
#
#  @api
#  Scenario: Get a project
#    Given I am logged in as a user with the "administrator" role
#    Given I am viewing a project named "demo" with the git url "http://github.com/jonpugh/drupal.git"
#    Then I should see "Project Name"
#    And I should see "demo"
#    And I should see "http://github.com/jonpugh/drupal.git"
#
#    Then break


# @TODO: Commented out until the drush user-create bug is fixed.
#Feature: Environment settings save.
#  @TODO: Only works if you have a project named "drupal".  Coming Soon.
#  Scenario: Check environment settings.
#    Given I am on the homepage
#    And I fill in "jon" for "Username"
#    And I fill in "jon" for "Password"
#    Then I press "Log in"
#
#    When I am at "project/drupal"
#    Then I click "Environment Settings"
#    When I fill in "NewUsername" for "Username"
#    And I fill in "NewPassword" for "Password"
#    Then I press "Save"
##    Then I should see "Environment settings saved for dev in project drupal"
#
#    When I click "Environment Settings"
#    Then the field "Username" should have the value "NewUsername"
#    When I fill in "" for "Username"
#    And I fill in "" for "Password"
#    Then I press "Save"
#
#    When I click "Environment Settings"
#    Then the field "Username" should have the value ""
