Feature: Anonymous Homepage

  Scenario: The homepage works

    Given I am on the homepage
    Then I should see "Sign In"
    And I should see "Username"
    And I should see "Password"
    When I fill in "wrong" for "Username"
    And I press "Log in"
    Then I should see "Password field is required."
    And I should see "Sorry, unrecognized username or password."

    When I click "Forgot your Password?"
    Then I should see "Forgot your password?"
    And I fill in "wrong" for "Username or e-mail address"
    And I press "E-mail new password"
    Then I should see "Sorry, wrong is not recognized as a user name or an e-mail address."

  @api
  Scenario:   The homepage works when devshop support is enabled.
    When I run drush "vset devshop_support_license_key tests"
    Then print last drush output
    When I run drush "vset devshop_support_license_key_status active"
    Then print last drush output

    Given I am on the homepage
    Then I should see "Sign In"
    And I should see "Username"
    And I should see "Password"
    And I should see "Welcome to devshop.local.computer! Please sign in using one of the following options:"
    And I should see the link "Sign in with DevShop.Support"

    When I am logged in as a user with the "authenticated user" role
    Then I should see "License Status: Active"