<?php

namespace Drupal\task\Element;

use Drupal\Core\Render\Element\RenderElement;
use SensioLabs\AnsiConverter\AnsiToHtmlConverter;
use SensioLabs\AnsiConverter\Theme\SolarizedXTermTheme;
use SensioLabs\AnsiConverter\Theme\Theme;

/**
 * Provides a render element to display an entity.
 *
 * Properties:
 * - #entity_type: The entity type.
 * - #entity_id: The entity ID.
 * - #view_mode: The view mode that should be used to render the entity.
 * - #langcode: For which language the entity should be rendered.
 *
 * Usage Example:
 * @code
 * $build['node'] = [
 *   '#type' => 'entity',
 *   '#entity_type' => 'node',
 *   '#entity_id' => 1,
 *   '#view_mode' => 'teaser,
 *   '#langcode' => 'en',
 * ];
 * @endcode
 *
 * @RenderElement("ansi_markup")
 */
class AnsiMarkup extends RenderElement {

  /**
   * {@inheritdoc}
   */
  public function getInfo() {
    return [
      '#pre_render' => [
        [get_class($this), 'preRenderAnsiMarkup'],
      ],
      '#view_mode' => 'full',
      '#langcode' => NULL,
    ];
  }

  /**
   * Entity element pre render callback.
   *
   * @param array $element
   *   An associative array containing the properties of the entity element.
   *
   * @return array
   *   The modified element.
   */
  public static function preRenderAnsiMarkup(array $element) {

    $theme = new Theme();
    $converter = new AnsiToHtmlConverter($theme, false);

    $element['output'] = [
      '#type' => 'container',
      '#attributes' => ['class' => 'task-module ansi-output'],
      '#attached' => ['library' => ['task/task']],
      'output' => [
        '#children' => $converter->convert($element['#output']),
      ],
      'style' => [
        '#type' => 'html_tag',
        '#tag' => 'style',
        '#children' => $theme->asCss(),
      ],
    ];
    return $element;
  }

}
