<?php

declare(strict_types=1);

namespace Drupal\site_hosting\Plugin\TaskType;

use Drupal\tasks\TaskTypePluginBase;

/**
 * Plugin implementation of the task_type.
 *
 * @TaskType(
 *   id = "verify_site",
 *   label = @Translation("Verify Site"),
 *   description = @Translation("Prepare code and launch site.")
 * )
 */
final class VerifySite extends TaskTypePluginBase {

  public function commands(): array
  {
    return [
      'git_checkout' => [],
      'ddev_start' => [],
    ];
  }

}
