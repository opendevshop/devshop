<?php

namespace Drupal\task\Form;

use Drupal\Core\Entity\BundleEntityFormBase;
use Drupal\Core\Entity\EntityTypeInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\task\Entity\TaskType;

/**
 * Form handler for task type forms.
 */
class TaskTypeForm extends BundleEntityFormBase {

  /**
   * {@inheritdoc}
   */
  public function form(array $form, FormStateInterface $form_state) {
    $form = parent::form($form, $form_state);

    /**
     * @var TaskType
     */
    $entity_type = $this->entity;

//    dsm($entity_type);
    if ($this->operation == 'edit') {
      $form['#title'] = $this->t('Edit %label task type', ['%label' => $entity_type->label()]);
    }

    $form['label'] = [
      '#title' => $this->t('Label'),
      '#type' => 'textfield',
      '#default_value' => $entity_type->label(),
      '#description' => $this->t('The human-readable name of this task type.'),
      '#required' => TRUE,
      '#size' => 30,
    ];

    $form['id'] = [
      '#type' => 'machine_name',
      '#default_value' => $entity_type->id(),
      '#maxlength' => EntityTypeInterface::BUNDLE_MAX_LENGTH,
      '#machine_name' => [
        'exists' => ['Drupal\task\Entity\TaskType', 'load'],
        'source' => ['label'],
      ],
      '#description' => $this->t('A unique machine-readable name for this task type. It must only contain lowercase letters, numbers, and underscores.'),
    ];

    $form['command_template'] = [
      '#title' => $this->t('Command Template'),
      '#type' => 'textfield',
      '#default_value' => $entity_type->commandTemplate(),
      '#description' => $this->t('The command to run, optionally using tokens to create a command from field data.'),
      '#required' => TRUE,
      '#size' => 30,
    ];

    $form['working_directory_template'] = [
      '#title' => $this->t('Working Directory Template'),
      '#type' => 'textfield',
      '#default_value' => $entity_type->workingDirectoryTemplate(),
      '#description' => $this->t('The directory to run the command in, optionally using tokens to create a command from field data.'),
      '#required' => TRUE,
      '#size' => 30,
    ];


    return $this->protectBundleIdElement($form);
  }

  /**
   * {@inheritdoc}
   */
  protected function actions(array $form, FormStateInterface $form_state) {
    $actions = parent::actions($form, $form_state);
    $actions['submit']['#value'] = $this->t('Save task type');
    $actions['delete']['#value'] = $this->t('Delete task type');
    return $actions;
  }

  /**
   * {@inheritdoc}
   */
  public function save(array $form, FormStateInterface $form_state) {

    /** @var TaskType $entity_type */
    $entity_type = $this->entity;

    $entity_type->set('id', trim($entity_type->id()));
    $entity_type->set('label', trim($entity_type->label()));
    $entity_type->set('command_template', $entity_type->commandTemplate());
    $entity_type->set('working_directory_template', $entity_type->workingDirectoryTemplate());

    $status = $entity_type->save();

    $t_args = ['%name' => $entity_type->label()];
    if ($status == SAVED_UPDATED) {
      $message = $this->t('The task type %name has been updated.', $t_args);
    }
    elseif ($status == SAVED_NEW) {
      $message = $this->t('The task type %name has been added.', $t_args);
    }
    $this->messenger()->addStatus($message);

    $form_state->setRedirectUrl($entity_type->toUrl('collection'));
  }

}
