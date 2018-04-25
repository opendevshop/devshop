<?php

use Drupal\DrupalExtension\Context\DrushContext;
use Behat\Behat\Context\SnippetAcceptingContext;
use Behat\Gherkin\Node\PyStringNode;
use Behat\Gherkin\Node\TableNode;
use Symfony\Component\Process\Process;
use Behat\Behat\Context\Context;
use Behat\Behat\Hook\Scope\BeforeScenarioScope;
use Behat\Testwork\Hook\Scope\AfterSuiteScope;

/**
 * Defines application features from the specific context.
 */
class FeatureContext extends DrushContext implements SnippetAcceptingContext {
  
  
  /**
   * Make MinkContext available.
   * @var \Drupal\DrupalExtension\Context\MinkContext
   */
  private $minkContext;
  
  /**
   * Prepare Contexts.
   * @BeforeScenario
   */
  public function gatherContexts(BeforeScenarioScope $scope)
  {
    $environment = $scope->getEnvironment();
    $this->minkContext = $environment->getContext('Drupal\DrupalExtension\Context\MinkContext');
  }
  
  /**
   * Log output and watchdog logs after any failed step.
   * @AfterStep
   */
  public function logAfterFailedStep($event)
  {
    if ($event->getTestResult()->getResultCode() === \Behat\Testwork\Tester\Result\TestResult::FAILED) {
      
      // Print Current URL and Last reponse after any step failure.
      echo "Step Failed.";
      
      echo "\nLasts Response:\n";
      $this->minkContext->printLastResponse();
      
      echo "\nWatchdog Errors:\n";
      $this->assertDrushCommand('wd-show');
      $this->printLastDrushOutput();
      
    }
  }
  
  /**
   * Initializes context.
   *
   * Every scenario gets its own context instance.
   * You can also pass arbitrary arguments to the
   * context constructor through behat.yml.
   */
  public function __construct() {
  }

  /**
   * @Then I wait :seconds seconds
   */
  public function iWaitSeconds($seconds)
  {
    sleep($seconds);
  }

  /**
   * @Then save last response
   */
  public function saveLastResponse()
  {
//
//    $path = '/var/aegir/devmaster-0.x/sites/devshop.site/files/test-output.html';
//
//    $file = file_save_data($this->getSession()->getPage()->getContent(), $path);
//
//    $link = str_replace('/var/aegir/devmaster-0.x/sites/devshop.site/files/', 'http://devshop.site/sites/devshop.site/files/', $file);
//    echo "Saved output to $link";
  }

  /**
   * Creates a project.
   *
   * @Given I am viewing a project named :title with the git url :git_url
   */
  public function createProject($title, $git_url) {
    $node = (object) array(
        'title' => $title,
        'type' => 'project',
        'project' => (object) array(
          'git_url' => $git_url,
          'install_profile' => 'standard',
          'settings' => (object) array(
            'git' => array(),
          ),
        ),
    );
    $saved = $this->nodeCreate($node);

    // Set internal page on the new node.
    $this->getSession()->visit($this->locatePath('/node/' . $saved->nid));
  }

  /**
   * @Then then field :field should have the value :value
   */
  public function thenFieldShouldHaveTheValue($field, $value)
  {
    $field = $this->fixStepArgument($field);
    $value = $this->fixStepArgument($value);

    $field_object = $this->getSession()->getPage()->findField($field);

    if (null === $field_object) {
      throw new \Exception('No field found with id|name|label|value ' . $field);
    }

    if ($field_object->getAttribute('value') != $value) {
      $current_value = $field_object->getAttribute('value');
      throw new \Exception("The field '$field' has the value '$current_value', not '$value'.");
    }
  }

  /**
   * Returns fixed step argument (with \\" replaced back to ").
   *
   * A copy from MinkContext
   *
   * @param string $argument
   *
   * @return string
   */
  protected function fixStepArgument($argument)
  {
    return str_replace('\\"', '"', $argument);
  }

  /**
   * @When I select the radio button with the label :label
   *
   * @TODO: Remove when PR is merged: https://github.com/jhedstrom/drupalextension/pull/302
   */
  public function assertSelectRadioByName($label) {
    $element = $this->getSession()->getPage();
    $radiobutton = $element->find('named', array('radio', $this->getSession()->getSelectorsHandler()->xpathLiteral($label)));
    if ($radiobutton === NULL) {
      throw new \Exception(sprintf('The radio button with the label "%s" was not found on the page %s', $label, $this->getSession()->getCurrentUrl()));
    }
    $value = $radiobutton->getAttribute('value');
    $radiobutton->selectOption($value, FALSE);
  }
  
  /**
   * @AfterSuite
   */
  public static function deleteProjectCode(AfterSuiteScope $scope)
  {
    print "Deleting /var/aegir/projects/drpl8";
    print shell_exec('rm -rf /var/aegir/projects/drpl8');
  }

    /**
     *
     * @When I submit a pull-request
     */
    public function iSubmitAPullRequest()
    {
        $url = $this->minkContext->getSession()->getPage()->findField('webhook-url')->getAttribute('value');
        $postdata = file_get_contents(dirname(dirname(__FILE__)) . '/assets/pull-request-opened.json');

        print "Found WebHook URL: $url";

        // This one works, let's test for a URL first.
//        $this->getDriver('drush')->getClient()->request ('POST', $url, $postdata);

        if (empty($url)) {
            throw new \Exception('Unable to find webhook URL.');
        }
    }
}
