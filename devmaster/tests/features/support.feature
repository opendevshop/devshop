Feature: Anonymous Homepage

  @api
  Scenario:   The homepage works when devshop support is enabled.
    Given I am on the homepage
    Then I should see "Sign In"
    And I should see "Username"
    And I should see "Password"
    And I should not see "Please sign in using one of the following options:"
    And I should not see the link "Sign in with DevShop.Cloud"

    When I am logged in as a user with the "authenticated user" role

# @TODO: When finalizing the devshop cloud client module, re-implement these tests.
#    Then I should see "Your DevShop server is currently unsupported."
#    When I run drush "vset devshop_support_license_key automated_testing_license_key"
#    # Then print last drush output
#    When I run drush "vset devshop_support_license_key_status active"
#    # Then print last drush output
#
#    Then I am on the homepage
#    Then I should not see "Your DevShop server is currently unsupported."
#    Then I should see "License Status: Active"
#
#    When I am logged in as a user with the "administrator" role
#    Given I am at "admin/devshop/support"
#    Then I should see "DevShop.Support"
#    And the "DevShop Support License Key" field should contain "automated_testing_license_key"