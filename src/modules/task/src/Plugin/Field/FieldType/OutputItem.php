<?php

namespace Drupal\task\Plugin\Field\FieldType;

use Drupal\Component\Utility\Random;
use Drupal\Core\Field\FieldDefinitionInterface;
use Drupal\Core\Field\FieldItemBase;
use Drupal\Core\Field\FieldStorageDefinitionInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\TypedData\DataDefinition;
use Symfony\Component\Process\Process;

/**
 * Defines the 'output' field type.
 *
 * @FieldType(
 *   id = "output",
 *   label = @Translation("Command Output"),
 *   category = @Translation("General"),
 *   default_formatter = "output_ansi"
 * )
 */
class OutputItem extends FieldItemBase {

  /**
   * {@inheritdoc}
   */
  public function isEmpty() {
    if ($this->output !== NULL) {
      return FALSE;
    }
    elseif ($this->stream !== NULL) {
      return FALSE;
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public static function propertyDefinitions(FieldStorageDefinitionInterface $field_definition) {

    $properties['output'] = DataDefinition::create('string')
      ->setLabel(t('Output'));
    $properties['stream'] = DataDefinition::create('integer')
      ->setLabel(t('Stream'));

    return $properties;
  }

  /**
   * {@inheritdoc}
   */
  public function getConstraints() {
    $constraints = parent::getConstraints();

    $options['stream']['AllowedValues'] = array_keys(OutputItem::allowedStreamValues());

    $options['stream']['NotBlank'] = [];

    $constraint_manager = \Drupal::typedDataManager()->getValidationConstraintManager();
    $constraints[] = $constraint_manager->create('ComplexData', $options);
    // @todo Add more constraints here.
    return $constraints;
  }

  /**
   * {@inheritdoc}
   */
  public static function schema(FieldStorageDefinitionInterface $field_definition) {

    $columns = [
      'output' => [
        'type' => 'text',
        'size' => 'big',
      ],
      'stream' => [
        'type' => 'int',
        'size' => 'normal',
      ],
    ];

    $schema = [
      'columns' => $columns,
      // @DCG Add indexes here if necessary.
    ];

    return $schema;
  }

  /**
   * {@inheritdoc}
   */
  public static function generateSampleValue(FieldDefinitionInterface $field_definition) {

    $random = new Random();

    $values['output'] = $random->paragraphs(1);
    $values['stream'] = Process::STDOUT;

    return $values;
  }

  /**
   * Returns allowed values for 'stream' sub-field.
   *
   * @return array
   *   The list of allowed values.
   */
  public static function allowedStreamValues() {
    return [
      Process::STDOUT => 'stdout',
      Process::STDERR => 'stderr',
    ];
  }

}
