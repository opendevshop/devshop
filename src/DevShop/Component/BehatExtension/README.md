# DevShop Drupal Behat Extension

This Behat extension simply extends the [Drupal Behat Extension](https://github.com/jhedstrom/drupalextension) to make it a bit more useful.

It does not require DevShop, but some features are even better when using DevShop.

## New Steps

### Then I take a screenshot

This step, when used with a Selenium server and the test has a "@javascript" tag, will save a screenshot to the active website's files folder and print a link.

### When I run "command"

This step simply runs `exec()` on any command and throws an exception if a non-zero exit code is thrown. 

### I select the radio button with a label that contains the string :label

This is very helpful for radio buttons that have a long or dynamic Label. Searches for the string instead of doing an exact match.

## Other Features 

### Simplified setup.

This plugin is now included in the [DevShop Composer Template](https://github.com/opendevshop/devshop-composer-template), which is the easiest way to get a Drupal codebase running in DevShop.

If you already have a project, you can copy the entire contents of [the `tests` folder](https://github.com/opendevshop/devshop-composer-template/tree/8.x/tests) into your project.

To start a new project, use the command:

```
composer create-project devshop/composer-template:8.x-dev --stability dev --no-interaction mynewproject
```



### Step Failure Information

When any step fails, the test runner outputs the URL it was on, it saves the HTML and a screenshot to web-readable folders, and gives you a link, like so:

```gherkin
Feature: DevShop.Support Registration
  In order to use DevShop.Support
  As a customer
  I need to register a new account.

  @api @javascript
  Scenario: Pre-beta registration                                     # features/demos.feature:7
    Given I am an anonymous user                                      # Drupal\DrupalExtension\Context\DrupalContext::assertAnonymousUser()
    When I am on "home"                                               # Drupal\DrupalExtension\Context\MinkContext::visit()
    And I take a screenshot                                           # FeatureContext::iTakeAScreenshot()
      │ Screenshot: http://pr35.devshop.support/sites/pr35.devshop.support/files//screenshot0U4Ddt.png 
      │ 
    Then I should see "Host Your Own Drupal"                          # Drupal\DrupalExtension\Context\MinkContext::assertPageContainsText()
      Element not found with xpath, //html
       (WebDriver\Exception\NoSuchElement)
    │
    │  Step Failed. 
    │  Site: pr35.devshop.support 
    │  Current URL: http://pr35.devshop.support/home
    │  Screenshot: http://pr35.devshop.support/sites/pr35.devshop.support/files/test_failures//IshouldseeHostYourOwnDrupalNLpp17.png 
    │  Last Page Output: http://pr35.devshop.support/sites/pr35.devshop.support/files/test_failures/failure-1539455962.html 
    │  
    │  Watchdog Errors:
    │   ID     Date          Type      Severity  Message                               
    │   42785  13/Oct 14:39  page not  warning   favicon.ico                           
    │                        found                                                     
    │   42784  13/Oct 14:39  page not  warning   favicon.ico                           
    │                        found                                                     
    │   42783  13/Oct 14:39  actions   info      5 orphaned actions                    
    │                                            (comment_publish_action,              
    │                                            comment_save_action,        
```

Many more features are in the works!
