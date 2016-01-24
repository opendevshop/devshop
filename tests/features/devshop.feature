Feature: Anonymous Homepage

  Scenario: The homepage works

    Given I am on the homepage
    Then I should see "Welcome to DevShop"
    And I should see "Enter your DevShop username"