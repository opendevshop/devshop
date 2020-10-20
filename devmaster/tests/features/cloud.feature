Feature: DevShop Cloud & Ansible Modules
  In order to build my own cloud
  As devshop user
  I need to create and destroy servers of any kind.

  @api
  Scenario: Enable DevShop Cloud: digitalocean
    Given I am logged in as a user with the "administrator" role
    When I am at "admin/hosting"
    And I check the box "DigitalOcean Cloud Servers"
    And I press "Save configuration"
    Then I should see the success message "Enabling Packet.net Cloud Servers feature."


  Scenario: Configure DevShop Cloud: digitalocean
    Given I am logged in as a user with the "administrator" role
    When I am at "admin/hosting"
    Then I click "Cloud Providers"
    And I fill in "xyzBadKey" for "DigitalOcean API Key"
    And I press "Save configuration"
    Then I should see the error message "DigitalOcean API Key failed validation"
