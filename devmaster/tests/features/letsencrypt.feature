Feature: DevShop Servers have LetsEncrypt enabled out of the box.
  In order to have a safe and secure website
  As devshop user
  I need to enable HTTPS as easily as possible

  @api
  Scenario: Server node has Certificate and HTTPS services enabled.

    Given I am logged in as a user with the "administrator" role
    When I am at "hosting/c/server_master"
    And I click "Edit"
    # Intentional failure. Fix in next commit.
    Then I should see "Let's Encrypt"
    Then I select the radio button with the label "LetsEncrypt"
    Then I select the radio button with the label "Staging"
#    Then I select the radio button with the label "Apache HTTPS"
    And I press "Save"
    When I run drush "hosting-tasks --force --fork=0 --strict=0"
    # Then print last drush output
    And I click "Servers"
    And I should see "Certificate"
    And I should see "LetsEncrypt"
#    And I should see "Apache HTTPS"

  @api
  Scenario: Server node has Certificate and HTTPS services enabled.
    Given I am logged in as a user with the "administrator" role
    When I am at "hosting/c/hostmaster"
    Then I should see "Encryption Disabled"