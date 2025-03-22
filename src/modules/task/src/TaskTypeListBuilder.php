<?php

namespace Drupal\task;

use Drupal\Core\Config\Entity\ConfigEntityListBuilder;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Url;use Drupal\task\Entity\TaskType;

/**
 * Defines a class to build a listing of task type entities.
 *
 * @see \Drupal\task\Entity\TaskType
 */
class TaskTypeListBuilder extends ConfigEntityListBuilder {

  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    $header['title'] = $this->t('Type');
    $header['command'] = $this->t('Command');
    $header['description'] = $this->t('Description');

    return $header + parent::buildHeader();
  }

  /**
   * {@inheritdoc}
   */
  public function buildRow(EntityInterface $entity) {
    $row['title'] = [
      'data' => $entity->label(),
      'class' => ['menu-label'],
    ];
//    $row['command'] = $entity->command();
//    $row['command'] = $entity->description();

    return $row + parent::buildRow($entity);
  }

  /**
   * {@inheritdoc}
   */
  public function render() {
    $build = parent::render();

    $build['table']['#empty'] = $this->t(
      'No task types available. <a href=":link">Enable a Task Type module</a> to continue.',
      [':link' => Url::fromRoute('system.modules_list')->toString()]
    );

    return $build;
  }

}
