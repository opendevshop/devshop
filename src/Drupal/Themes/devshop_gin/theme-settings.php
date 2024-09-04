<?php

declare(strict_types=1);

/**
 * @file
 * Theme settings form for DevShop Gin theme.
 */

use Drupal\Core\Form\FormState;

/**
 * Implements hook_form_system_theme_settings_alter().
 */
function devshop_gin_form_system_theme_settings_alter(array &$form, FormState $form_state): void {

  $form['devshop_gin'] = [
    '#type' => 'details',
    '#title' => t('DevShop Gin'),
    '#open' => TRUE,
  ];

  $form['devshop_gin']['example'] = [
    '#type' => 'textfield',
    '#title' => t('Example'),
    '#default_value' => theme_get_setting('example'),
  ];

}
