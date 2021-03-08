<?php

namespace DevShop\Behat\DrupalExtension\Context;

use Behat\Mink\Exception\UnsupportedDriverActionException;
use Drupal\DrupalExtension\Context\RawDrupalContext;
//use Behat\MinkExtension\Context\RawMinkContext;
//use Behat\Testwork\Hook\HookDispatcher;
//
//use Drupal\DrupalDriverManager;

//use Drupal\DrupalExtension\Hook\Scope\AfterLanguageEnableScope;
//use Drupal\DrupalExtension\Hook\Scope\AfterNodeCreateScope;
//use Drupal\DrupalExtension\Hook\Scope\AfterTermCreateScope;
//use Drupal\DrupalExtension\Hook\Scope\AfterUserCreateScope;
//use Drupal\DrupalExtension\Hook\Scope\BaseEntityScope;
//use Drupal\DrupalExtension\Hook\Scope\BeforeLanguageEnableScope;
//use Drupal\DrupalExtension\Hook\Scope\BeforeNodeCreateScope;
//use Drupal\DrupalExtension\Hook\Scope\BeforeUserCreateScope;
//use Drupal\DrupalExtension\Hook\Scope\BeforeTermCreateScope;

use Behat\Behat\Hook\Scope\BeforeScenarioScope;
use Behat\Behat\Hook\Scope\AfterStepScope;
use Behat\Behat\Hook\Scope\AfterScenarioScope;

/**
 * Provides the raw functionality for interacting with Drupal.
 */
class DevShopDrupalContext extends RawDrupalContext {

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
     * @var array List of URLs the link test step has clicked.
     */
    private $visitedLinks = array();

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
            $alias = ltrim($drush_config['alias'], '@');

            // If there is no drush alias in the behat yml config, don't do anything special, but do show a message.
            if (empty($alias)) {
              print "DevShopDrupalContext Warning: No drush alias found in behat yml config.";
              return;
            }

            // Detect drupal version
            $drupal_version = trim(shell_exec("drush @$alias status --fields=drupal-version"));
            if (strpos($drupal_version, '8.') !== FALSE) {
              $php_cmd = escapeshellarg('print \Drupal::service("file_system")->realpath(file_default_scheme() . "://")');
              $cmd = "drush @$alias eval $php_cmd";
            }
            else {
              $cmd = "drush @$alias vget file_public_path --format=string";
            }

            // Lookup file_directory_path
            $files_path = trim(shell_exec($cmd));

            // Check for various problems.
            if (empty($files_path)) {
                throw new \Exception("Results of command '$cmd' was empty. Check your settings and try again.'");
            }
            elseif (!file_exists($files_path)) {
                throw new \Exception("Unable to find directory at 'files_path' Drupal parameter '$files_path'");
            }
            elseif (!is_writable($files_path)) {
                throw new \Exception("Files path '$files_path' is not writable. Check permissions and try again.");
            }

            $output_directory = $files_path .'/test_failures';
            $output_path = $files_path .'/test_failures/failure-' . time() . '.html';

            // Print Current URL and Last reponse after any step failure.
            echo "Step Failed. \n";
            echo "Site: $alias \n";
            echo "Current URL: " . $this->getSession()->getCurrentUrl() . "\n";

            if (!file_exists($output_directory)) {
                mkdir($output_directory);
            }
            $wrote = file_put_contents($output_path, $this->getSession()->getPage()->getContent());

            $this->takeAScreenshot($event->getStep()->getText(), $output_directory, $base_url);
            //
            //        if ($this->getSession()->getDriver() instanceof \Behat\Mink\Driver\Selenium2Driver) {
            //            $stepText = $event->getStep()->getText();
            //            $fileName = preg_replace('#[^a-zA-Z0-9\._-]#', '', $stepText);
            //
            //            // Get filename by getting tmpname, then stripping output directory.
            //            $full_output_dir = realpath($output_directory);
            //            $fileName = str_replace($full_output_dir, '', tempnam($full_output_dir, $fileName) . '.png');
            //            $this->saveScreenshot($fileName, $full_output_dir);
            //            echo "Screenshot: $base_url/$output_directory/$fileName \n";
            //        }


            if (isset($_SERVER['TRAVIS'])) {
                echo "\nLasts Response:\n";
                $this->minkContext->printLastResponse();
            }
            else {

                if ($wrote) {
                    echo "Last Page Output: $base_url/$output_path \n";
                }
                else {
                    throw new \Exception("Something failed when writing output to $base_url/$output_path ... \n");
                }
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
     * @When I save a screenshot
     */
    public function iTakeAScreenshot()
    {
        $base_url = $this->getMinkParameter('base_url');
        $drush_config = $this->drupalContext->getDrupalParameter('drush');
        $alias = ltrim($drush_config['alias'], '@');

        // Lookup file_directory_path
        $cmd = "drush @$alias vget file_public_path --format=string";
        $files_path = trim(shell_exec($cmd));

        $this->takeAScreenshot('screenshot', $files_path, $base_url);
    }

    private function takeAScreenshot($filename_string, $output_directory, $base_url) {

        if ($this->getSession()->getDriver() instanceof \Behat\Mink\Driver\Selenium2Driver) {
            $stepText = $filename_string;
            $fileName = preg_replace('#[^a-zA-Z0-9\._-]#', '', $stepText);

            // Get filename by getting tmpname, then stripping output directory.
            $full_output_dir = realpath($output_directory);
            $fileName = str_replace($full_output_dir, '', tempnam($full_output_dir, $fileName) . '.png');
            $this->saveScreenshot($fileName, $full_output_dir);
            echo "Screenshot: $base_url/$output_directory/$fileName \n";
        }
    }

    /**
     * @When I run :command
     */
    public function iRun($command)
    {
      exec($command, $output, $return_var);
      print implode("\n", $output);
      if ($return_var != 0) {
        throw new \Exception("The command $command returned a non-zero exit code.");
      }
    }

    /**
     * @When I select the radio button with a label that contains the string :label with the id :id
     * @When I select the radio button with a label that contains the string :label
     */
    public function assertSelectRadioByLabelString($label, $id = '') {
        $element = $this->getSession()->getPage();
        $radiobutton = $id ? $element->findById($id) : $element->find('named', array('radio', $this->getSession()->getSelectorsHandler()->xpathLiteral($label)));
        if ($radiobutton === NULL) {
           throw new \Exception(sprintf('The radio button with "%s" was not found on the page %s', $id ? $id : $label, $this->getSession()->getCurrentUrl()));
        }
        $value = $radiobutton->getAttribute('value');
        $radio_id = $radiobutton->getAttribute('id');
        $labelonpage = $element->find('css', "label[for='$radio_id']")->getText();
        if (strpos($labelonpage, $label) === FALSE) {
            throw new \Exception(sprintf("Label for button with id '%s' did not contain '%s' on the page %s.  Label: %s", $id, $$label, $this->getSession()->getCurrentUrl(), labelonpage));
        }
        $radiobutton->selectOption($value, FALSE);
    }

    /**
     * Visit a given path, and check for an arbitrary HTTP response code.
     *
     * @When I visit :path I should get the response code :response_code
     *
     * @throws UnsupportedDriverActionException
     */
    public function assertResponseAtPath($path, $response_code = 200) {
        $this->getSession()->visit($this->locatePath($path));

        // If available, add extra validation that this is a 200 response.
        try {
            $this->getSession()->getStatusCode();
            $this->minkContext->assertHttpResponse($response_code);
        }
        catch (UnsupportedDriverActionException $e) {
            // Simply continue on, as this driver doesn't support HTTP response codes.
        }
    }

    /**
     * Looks for every "a href" on the page and ensures a 200 code, skipping
     * anchors, mailto, and external links.
     *
     * Originally created at https://gist.github.com/jonpugh/b2f95dd8e89b3218a20a
     *
     * @author Jon Pugh <jon@thinkdrop.net>
     * @Then every link should work
     * @Then every link on the page should work
     */
    public function everyLinkShouldWork() {
        $elements = $this->getSession()
          ->getPage()
          ->findAll('xpath', '//a/@href');
        $count = count($elements);
        foreach ($elements as $element) {
            // If element or tag is empty...
            if (empty($element->getParent())) {
                continue;
            }
            $href = $element->getParent()->getAttribute('href');

            // Skip if empty
            if (empty($href)) {
                continue;
            }
            // Skip if already visited
            if (isset($this->visitedLinks[$href])) {
                print "Skipping visited link: $href \n\n";
                continue;
            }
            // Save URL for later to avoid duplicates.
            $this->visitedLinks[$href] = $href;
            // Skip if an anchor tag
            if (strpos($href, '#') === 0) {
                print "Skipping anchor link: $href \n\n";
                continue;
            }
            // Skip remote links
            if (strpos($href, 'http') === 0) {
                print "Skipping remote link: $href  \n\n";
                continue;
            }
            // Skip mailto links
            if (strpos($href, 'mailto') === 0) {
                print "Skipping remote link: $href  \n\n";
                continue;
            }
            print "Checking Link: " . $href . "\n";
            $this->assertResponseAtPath($href, 200);

            // @TODO: Remove this once we know it works.
            // Mimics Drupal\DrupalExtension\Context\MinkContext::assertAtPath
            //            $this->getSession()->visit($this->locatePath($href));
            //            try {
//                $this->getSession()->getStatusCode();
//                $this->assertSession()->statusCodeEquals('200');
//                print "200 Success \n";
//            } catch (UnsupportedDriverActionException $e) {
//                // Simply continue on, as this driver doesn't support HTTP response codes.
//            } catch (ExpectationException $e) {
//                print "200 Success NOT received \n";
//                throw new \Exception("Page at URL $href did not return 200 code.");
//            } catch (Behat\Mink\Exception\DriverException $e) {
//                throw new \Exception($e->getMessage());
//            }
            print "\n";
        }
        print "Done! Checked $count Links";
    }
}
