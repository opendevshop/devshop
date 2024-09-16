<?php

namespace Drupal\commands\Plugin\Command;

use Drupal\commands\CommandPluginBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\user\Entity\User;

/**
 * Plugin implementation of the task_command.
 *
 * @Command(
 *   id = "composer_install",
 *   label = @Translation("Composer Install"),
 *   description = @Translation("Runs composer install."),
 *   command = "composer install --ansi"
 * )
 */
class ComposerInstall extends CommandPluginBase {

  public function command($params = []) {
    if (!empty($params['verbose'])) {
      return parent::command($params) . ' --verbose';
    }
    return parent::command($params);
  }

  /**
   * Show a form on the "Run command" form.
   * @return void
   */
  public function form($form, FormStateInterface $form_state) {
    $form['parameters']['verbose'] = [
      '#type' => 'checkbox',
      '#title' => t('Verbose'),
      '#description'=> t('Get verbose information.'),
    ];

    // @TODO: Reset working_directory to detected composer root.
    return $form;
  }
}
