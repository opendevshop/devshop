<?php

namespace Drupal\task\Entity;

use Drupal\Core\Entity\ContentEntityBase;
use Drupal\Core\Entity\EntityChangedTrait;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Entity\EntityTypeInterface;
use Drupal\Core\Entity\RevisionableContentEntityBase;
use Drupal\Core\Field\BaseFieldDefinition;
use Drupal\Core\Field\FieldStorageDefinitionInterface;
use Drupal\task\TaskInterface;
use Drupal\user\EntityOwnerTrait;

/**
 * Defines the task entity class.
 *
 * @ContentEntityType(
 *   id = "task",
 *   label = @Translation("Task"),
 *   label_collection = @Translation("Tasks"),
 *   label_singular = @Translation("task"),
 *   label_plural = @Translation("tasks"),
 *   label_count = @PluralTranslation(
 *     singular = "@count tasks",
 *     plural = "@count tasks",
 *   ),
 *   bundle_label = @Translation("Task type"),
 *   handlers = {
 *     "list_builder" = "Drupal\task\TaskListBuilder",
 *     "views_data" = "Drupal\views\EntityViewsData",
 *     "access" = "Drupal\task\TaskAccessControlHandler",
 *     "form" = {
 *       "add" = "Drupal\task\Form\TaskForm",
 *       "edit" = "Drupal\task\Form\TaskForm",
 *       "delete" = "Drupal\Core\Entity\ContentEntityDeleteForm",
 *     },
 *     "route_provider" = {
 *       "html" = "Drupal\Core\Entity\Routing\AdminHtmlRouteProvider",
 *     }
 *   },
 *   base_table = "operations_task",
 *   data_table = "operations_task_data",
 *   revision_table = "operations_task_revision",
 *   revision_data_table = "operations_task_revision_data",
 *   show_revision_ui = TRUE,
 *   admin_permission = "administer task types",
 *   entity_keys = {
 *     "id" = "id",
 *     "bundle" = "task_type",
 *     "label" = "id",
 *     "revision" = "vid",
 *     "uuid" = "uuid",
 *     "owner" = "uid",
 *   },
 *   links = {
 *     "collection" = "/admin/content/task",
 *     "add-form" = "/task/run/{task_type}",
 *     "add-page" = "/task/run",
 *     "canonical" = "/task/{task}",
 *     "edit-form" = "/task/{task}/edit",
 *     "delete-form" = "/task/{task}/delete",
 *      "version_history" = "/task/{site}/history",
 *      "revision" = "/task/{site}/history/{task_revision}/view",
 *   },
 *   revision_metadata_keys = {
 *     "revision_user" = "revision_uid",
 *     "revision_created" = "revision_timestamp",
 *     "revision_log_message" = "revision_log",
 *   },
 *   bundle_entity_type = "task_type",
 *   field_ui_base_route = "entity.task_type.edit_form",
 * )
 */
class Task extends RevisionableContentEntityBase {

  use EntityChangedTrait;
  use EntityOwnerTrait;

  /**
   * A task is queued.
   */
  const TASK_QUEUED = -1;

  /**
   * A task process is running.
   */
  const TASK_PROCESSING = 3;

  /**
   * The task ended with an error.
   */
  const TASK_ERROR = 2;

  /**
   * The task ended successfully.
   */
  const TASK_OK = 0;

  /**
   * The task ended but a warning was detected.
   */
  const TASK_WARN = 1;

  /**
   * Human-readable strings for state.
   *
   * @var string
   */
  const STATE_NAMES = [
    self::TASK_OK => 'OK',
    self::TASK_WARN => 'Warning',
    self::TASK_ERROR => 'Error',
    self::TASK_PROCESSING => 'Processing',
    self::TASK_QUEUED => 'Queued',
  ];

  /**
   * {@inheritdoc}
   */
  public function preSave(EntityStorageInterface $storage) {
    parent::preSave($storage);
    if (!$this->getOwnerId()) {
      // If no owner has been set explicitly, make the anonymous user the owner.
      $this->setOwnerId(0);
    }

    // Parse command from task type command template.
    // @TODO: Pass through tokenization.
    $command = $this->command->value;
    $this->set('command', $command);


    // Parse working directory from task type command template.
    // @TODO: Pass through tokenization.
    $working_directory = $this->working_directory->value;
    $this->set('working_directory', $working_directory);

    // If "finished" time is set, save duration field.
    if ($this->finished->value) {
      $this->duration->setValue($this->finished->value - $this->executed->value);
    }

  }

  /**
   * {@inheritdoc}
   */
  public static function baseFieldDefinitions(EntityTypeInterface $entity_type) {

    $fields = parent::baseFieldDefinitions($entity_type);

    $fields['uid'] = BaseFieldDefinition::create('entity_reference')
      ->setLabel(t('Author'))
      ->setSetting('target_type', 'user')
      ->setDefaultValueCallback(static::class . '::getDefaultEntityOwner')
      ->setDisplayOptions('form', [
        'type' => 'entity_reference_autocomplete',
        'settings' => [
          'match_operator' => 'CONTAINS',
          'size' => 60,
          'placeholder' => '',
        ],
        'weight' => 15,
      ])
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayOptions('view', [
        'label' => 'above',
        'type' => 'author',
        'weight' => 15,
      ])
      ->setDisplayConfigurable('view', TRUE);

    $fields['created'] = BaseFieldDefinition::create('created')
      ->setLabel(t('Queued on'))
      ->setDescription(t('The time that the task entity was created.'))
      ->setDisplayOptions('view', [
        'label' => 'above',
        'type' => 'timestamp',
        'weight' => 20,
      ])
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayOptions('form', [
        'type' => 'datetime_timestamp',
        'weight' => 20,
      ])
      ->setDisplayConfigurable('view', TRUE);

    $fields['state'] = BaseFieldDefinition::create('list_integer')
      ->setSetting('allowed_values', [
        static::STATE_NAMES
      ])
      ->setLabel(t('Task State'))
      ->setDescription(t('The latest state of a task.'))
      ->setDefaultValue(static::TASK_QUEUED)
      ->setRequired(TRUE)
      ->setRevisionable(TRUE)
      ->setDefaultValue(static::TASK_QUEUED)
      ->setDisplayConfigurable('view', TRUE)
    ;

    $fields['command'] = BaseFieldDefinition::create('string')
      ->setLabel(t('Command'))
      ->setRequired(TRUE)
      ->setTranslatable(TRUE)
      ->setRevisionable(TRUE)
      ->setDisplayOptions('view', [
        'label' => 'visible',
        'type' => 'string',
        'weight' => -5,
      ])
      ->setDisplayConfigurable('view', TRUE)
    ;
    $fields['working_directory'] = BaseFieldDefinition::create('string')
      ->setLabel(t('Working Directory'))
      ->setDescription(t('The directory to run the command in.'))
      ->setDisplayOptions('view', [
        'label' => 'visible',
        'type' => 'string',
        'weight' => -5,
      ])
      ->setDisplayConfigurable('view', TRUE)
    ;

    $fields['output'] = BaseFieldDefinition::create('output')
      ->setLabel(t('Output'))
      ->setDescription(t('Output from the command.'))
      ->setCardinality(FieldStorageDefinitionInterface::CARDINALITY_UNLIMITED)
      ->setRequired(FALSE)
      ->setTranslatable(FALSE)
      ->setRevisionable(FALSE)
      ->setDisplayOptions('view', [
        'label' => 'visible',
        'type' => 'output_ansi',
        'weight' => -5,
      ])
      ->setDisplayConfigurable('view', TRUE)
    ;
    $fields['executed'] = BaseFieldDefinition::create('timestamp')
      ->setLabel(t('Start Time'))
      ->setDescription(t('The time that the command was started.'))
      ->setRequired(FALSE)
      ->setTranslatable(FALSE)
      ->setRevisionable(FALSE)
      ->setDisplayOptions('view', [
        'label' => 'visible',
        'type' => 'timestamp',
        'weight' => -5,
      ])
      ->setDisplayConfigurable('view', TRUE)
    ;
    $fields['finished'] = BaseFieldDefinition::create('timestamp')
      ->setLabel(t('End Time'))
      ->setDescription(t('The time that the command ended.'))
      ->setRequired(FALSE)
      ->setTranslatable(FALSE)
      ->setRevisionable(FALSE)
      ->setDisplayOptions('view', [
        'label' => 'visible',
        'type' => 'timestamp',
        'weight' => -5,
      ])
      ->setDisplayConfigurable('view', TRUE)
    ;

    $fields['duration'] = BaseFieldDefinition::create('integer')
      ->setLabel(t('Duration'))
      ->setDescription(t('The amount of time that a command took to run.'))
      ->setRequired(FALSE)
      ->setSetting('suffix', t(' seconds'))
      ->setTranslatable(FALSE)
      ->setRevisionable(FALSE)
      ->setDisplayOptions('view', [
        'label' => 'visible',
        'type' => 'integer',
        'weight' => -5,
      ])
      ->setDisplayConfigurable('view', TRUE)
    ;

    return $fields;
  }
}
