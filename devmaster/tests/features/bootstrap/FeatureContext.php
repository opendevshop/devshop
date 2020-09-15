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
class FeatureContext extends \Drupal\DrupalExtension\Context\BatchContext implements SnippetAcceptingContext {

    /**
     * Make MinkContext available.
     * @var \Drupal\DrupalExtension\Context\MinkContext
     */
    private $minkContext;

    /**
     * Make DrupalContext available.
     * @var \Drupal\DrupalExtension\Context\DrupalContext
     */
    private $drupalContext;

    /**
     * Make MinkContext available.
     * @var \Drupal\DrupalExtension\Context\DrushContext
     */
    private $drushContext;

    /**
     * Prepare Contexts.
     * @BeforeScenario
     */
    public function gatherContexts(BeforeScenarioScope $scope)
    {
        $environment = $scope->getEnvironment();
        $this->minkContext = $environment->getContext('Drupal\DrupalExtension\Context\MinkContext');
        $this->drupalContext = $environment->getContext('Drupal\DrupalExtension\Context\DrupalContext');
        $this->drushContext = $environment->getContext('Drupal\DrupalExtension\Context\DrushContext');
    }

    /**
     * Log output and watchdog logs after any failed step.
     * @AfterStep
     */
    public function logAfterFailedStep($event)
    {
        if ($event->getTestResult()->getResultCode() === \Behat\Testwork\Tester\Result\TestResult::FAILED) {

            $base_url = $this->getMinkParameter('base_url');
            $drush_config = $this->drupalContext->getDrupalParameter('drush');
            $alias = $drush_config['alias'];

            // If environment variable is set, save assets to that.
            if (!empty(($_SERVER['DEVSHOP_TESTS_ASSETS_PATH']))) {
              if (is_writable($_SERVER['DEVSHOP_TESTS_ASSETS_PATH'])) {
                $files_path = $_SERVER['DEVSHOP_TESTS_ASSETS_PATH'];
                $output_notification_string = $files_path.'/output.html';
              }
              elseif (!file_exists($_SERVER['DEVSHOP_TESTS_ASSETS_PATH'])) {
                throw new \Exception("DEVSHOP_TESTS_ASSETS_PATH was set, but the directory does not exist. Change the environment variable or create the directory: " . $_SERVER['DEVSHOP_TESTS_ASSETS_PATH']);
              }
              else {
                throw new \Exception("DEVSHOP_TESTS_ASSETS_PATH was set, but is not writable: Change the environment variable or create the directory: " . $_SERVER['DEVSHOP_TESTS_ASSETS_PATH']);
              }
            }
            // If not, load the public writable files folder for devshop, so the asset can be served over HTTP.
            else {
              // Lookup file_directory_path
              $cmd = "drush @$alias vget file_public_path --format=string";
              $files_path = trim(shell_exec($cmd));
              $output_notification_string = "$base_url/$files_path/output.html";
            }

            // Check for various problems.
            if (empty($files_path)) {
                throw new \Exception("Unable to load files_public_path from devmaster: Results of command '$cmd' was empty.'");
            }
            elseif (!file_exists($files_path)) {
                throw new \Exception("Assets path not found: $files_path");
            }
            elseif (!is_writable($files_path)) {
                throw new \Exception("Assets path '$files_path' is not writable by the testing scipt and user.");
            }

            $output_path = $files_path .'/output.html';

            // Print Current URL and Last reponse after any step failure.
            echo "Step Failed. \n";
            echo "Site: $alias \n";
            echo "Current URL: " . $this->getSession()->getCurrentUrl() . "\n";

            if (!file_exists($files_path)) {
                mkdir($files_path);
            }
            $wrote = file_put_contents($output_path, $this->getSession()->getPage()->getContent());

            if ($wrote) {
                echo "Last Page Output Saved to: $output_notification_string \n";
            }
            else {
                throw new \Exception("Something failed when writing output to $output_path ... \n");
            }

            if (isset($_SERVER['CI'])) {
                echo "\nLasts Response:\n";
                $this->minkContext->printLastResponse();
            }

            echo "\nWatchdog Errors:\n";
            $this->drushContext->assertDrushCommand('wd-show');
            $this->drushContext->printLastDrushOutput();
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
   * @Then the field :field should have the value :value
   */
  public function theFieldShouldHaveTheValue($field, $value)
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
   * @AfterSuite
   */
  public static function deleteProjectCode(AfterSuiteScope $scope)
  {
    print "Deleting /var/aegir/projects/drpl8";
    print shell_exec('rm -rf /var/aegir/projects/drpl8');
    print shell_exec('rm -rf /var/aegir/config/server_master/apache/platform.d/platform_drpl8_*.conf');
  }

    /**
     *
     * @When I submit a pull-request
     */
    public function iSubmitAPullRequest()
    {
        $url = $this->minkContext->getSession()->getPage()->findField('webhook-url')->getAttribute('value');
        $postdata = file_get_contents(dirname(dirname(__FILE__)) . '/assets/pull-request-opened.json');

        if (empty($url)) {
            throw new \Exception('Unable to find webhook URL.');
        }

        print "Found WebHook URL: $url";

        // This one works, let's test for a URL first.
        $client = $this->getSession()->getDriver('drush')->getClient();

        /** @var Symfony\Component\DomCrawler\Crawler $response */
        $response = $client->request('POST', $url, [], [], [], $postdata);

        print_r($response->html());


    }
}
