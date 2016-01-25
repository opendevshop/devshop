#@api
#Feature: Create a project
#  In order to start developing a drupal site
#  As a project admin
#  I need to create a new project
#
#  Scenario: Create a new Project
#    Given I am logged in as a user with the "administrator" role
#    When I click "Projects"
#    And I click "Start a new Project"
#    Then I should see "Step 1"
#    Then I fill in "xxxxx" for "Project Code Name"
#    And I fill in "http://specialurl" for "Git URL"
#    When I press "Next"
#
#    # Project node form
#    Then I should see "xxxxx"
#    And I should see "http://specialurl"
#
#    # @TODO: Fill in all the settings.
#    Then I fill in "docroot" for "Path to Drupal"
#    Then I fill in "/var/aegir/projects/special" for "Code path"
#    Then I fill in "special" for "Base URL"
##    And I select the radio button "Manual Deployment"
#    And I check the box "Allow deploying data from drush aliases"
#    Then I fill in "live.com" for "Live Domain"
#    And I check the box "For new environments, create subdomains under Live Domain."
#    When I press "Next"
#
#    Then I should see "live.com"
#    And I should see "Path to Drupal: docroot"
#
#    And I should see "Default Stack"
#    And I should see "localhost" in the ".db-server-node" element
##    And I should see "devshop.site" in the ".web-server-node" element
#
#    # Go back and Edit
#    When I press "Back"
#    Then the "Path to Drupal" field should contain "docroot"
#    And the "Live Domain" field should contain "live.com"
#    And the "Code path" field should contain "/var/aegir/projects/special"
#    And the "Base URL" field should contain "special"
#    And the "Live Domain" field should contain "live.com"
#    And the "Allow deploying data from drush aliases" checkbox should be checked
#    And the "For new environments, create subdomains under Live Domain." checkbox should be checked
#
#    When I fill in "changedroot" for "Path to Drupal"
#    And I press "Next"
#    Then I should see "Path to Drupal: changedroot"
#
#    When I press "Cancel"
#    Then I should see "Project creation cancelled."
#    And I should be on "projects"
#
