Feature: Anonymous Homepage

  Scenario: The homepage works

    Given I am on the homepage
    Then I should see "Login"
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