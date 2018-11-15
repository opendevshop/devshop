Feature: DevShop Servers have LetsEncrypt enabled out of the box.
  In order to have a safe and secure website
  As devshop user
  I need to enable HTTPS as easily as possible

  @api
  Scenario: Server node has Certificate and HTTPS services enabled.

    Given I am logged in as a user with the "administrator" role
    And I click "Servers"
    And I should see "Certificate"
    And I should see "LetsEncrypt"
    And I should see "Apache HTTPS"

  @api
  Scenario: Server node has Certificate and HTTPS services enabled.
    Given I am logged in as a user with the "administrator" role
    When I am at "hosting/c/hostmaster"
    Then I should see "Encryption Disabled"