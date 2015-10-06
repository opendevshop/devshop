<?php

use Drupal\DrupalExtension\Context\RawDrupalContext;
use Behat\Behat\Context\SnippetAcceptingContext;
use Behat\Gherkin\Node\PyStringNode;
use Behat\Gherkin\Node\TableNode;

/**
 * Defines application features from the specific context.
 */
class FeatureContext extends RawDrupalContext implements SnippetAcceptingContext {

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
}
