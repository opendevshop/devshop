<?php

namespace Drupal\task\Plugin\Field\FieldFormatter;

use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Field\FormatterBase;
use Drupal\task\Plugin\Field\FieldType\OutputItem;

/**
 * Plugin implementation of the 'output_default' formatter.
 *
 * @FieldFormatter(
 *   id = "output_default",
 *   label = @Translation("Default"),
 *   field_types = {"output"}
 * )
 */
class OutputDefaultFormatter extends FormatterBase {

  /**
   * {@inheritdoc}
   */
  public function viewElements(FieldItemListInterface $items, $langcode) {
    $element = [];

    foreach ($items as $delta => $item) {

      if ($item->output) {
        $element[$delta]['output'] = [
          '#type' => 'item',
          '#title' => $this->t('Output'),
          '#markup' => $item->output,
        ];
      }

      if ($item->stream) {
        $allowed_values = OutputItem::allowedStreamValues();
        $element[$delta]['stream'] = [
          '#type' => 'item',
          '#title' => $this->t('Stream'),
          '#markup' => $allowed_values[$item->stream],
        ];
      }
    }

    return $element;
  }

}
