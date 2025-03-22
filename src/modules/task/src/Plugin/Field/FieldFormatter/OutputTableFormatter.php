<?php

namespace Drupal\task\Plugin\Field\FieldFormatter;

use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Field\FormatterBase;
use Drupal\task\Plugin\Field\FieldType\OutputItem;

/**
 * Plugin implementation of the 'output_table' formatter.
 *
 * @FieldFormatter(
 *   id = "output_table",
 *   label = @Translation("Table"),
 *   field_types = {"output"}
 * )
 */
class OutputTableFormatter extends FormatterBase {

  /**
   * {@inheritdoc}
   */
  public function viewElements(FieldItemListInterface $items, $langcode) {

    $header[] = '#';
    $header[] = $this->t('Output');
    $header[] = $this->t('Stream');

    $table = [
      '#type' => 'table',
      '#header' => $header,
    ];

    foreach ($items as $delta => $item) {
      $row = [];

      $row[]['#markup'] = $delta + 1;

      $row[]['#markup'] = $item->output;

      if ($item->stream) {
        $allowed_values = OutputItem::allowedStreamValues();
        $row[]['#markup'] = $allowed_values[$item->stream];
      }
      else {
        $row[]['#markup'] = '';
      }

      $row[]['#markup'] = $item->value_3;

      $table[$delta] = $row;
    }

    return [$table];
  }

}
