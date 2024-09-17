<?php

namespace Drupal\site_hosting\Plugin\Command;

use Drupal\commands\CommandPluginBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\user\Entity\User;

/**
 * Clones and checks out code.
 *
 * @Command(
 *   id = "ddev_start",
 *   label = @Translation("DDEV Start"),
 *   description = @Translation("Run ddev start."),
 *   command = {
 *     "ddev start"
 *   }
 * )
 */
class DdevStart extends CommandPluginBase {

}
