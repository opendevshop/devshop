<?php

namespace Drupal\task\Plugin\Field\FieldFormatter;

use Drupal\Core\Field\FieldItemListInterface;
use Drupal\Core\Field\FormatterBase;
use Drupal\Core\Form\FormStateInterface;
use SensioLabs\AnsiConverter\AnsiToHtmlConverter;
use SensioLabs\AnsiConverter\Theme\SolarizedXTermTheme;
use SensioLabs\AnsiConverter\Theme\Theme;

/**
 * Plugin implementation of the 'ANSI Output' formatter.
 *
 * @FieldFormatter(
 *   id = "output_ansi",
 *   label = @Translation("ANSI Color Output"),
 *   field_types = {
 *     "output",
 *   }
 * )
 */
class OutputAnsiFormatter extends FormatterBase {

  /**
   * {@inheritdoc}
   */
  public static function defaultSettings() {
    return [
      'theme' => 'default',
    ] + parent::defaultSettings();
  }

  /**
   * {@inheritdoc}
   */
  public function settingsForm(array $form, FormStateInterface $form_state) {

    $elements['theme'] = [
      '#type' => 'radios',
      '#title' => $this->t('Theme'),
      '#default_value' => $this->getSetting('theme'),
      '#options' => [
        'dark' => t('Dark'),
      ],
    ];

    return $elements;
  }

  /**
   * {@inheritdoc}
   */
  public function settingsSummary() {
    $summary[] = $this->t('Theme: @theme', ['@theme' => $this->getSetting('theme')]);
    return $summary;
  }

  /**
   * {@inheritdoc}
   */
  public function viewElements(FieldItemListInterface $items, $langcode) {
    $element = [];
    $lines = [];
    foreach ($items as $delta => $item) {
      $lines[] = $item->output;
    }
    $output = implode('', $lines);
    $element = [
      '#type' => 'ansi_markup',
      '#attributes' => ['class' => 'task-module'],
      '#output' => $output,
    ];
    return $element;
  }

}
