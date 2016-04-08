<?php

function ansible_roles_settings() {

  $rows = array();

  // Load all available roles
  $results = db_select('hosting_ansible_roles_available', 'h')
    ->fields('h')
    ->execute()
    ->fetchAllAssoc('name');

  foreach ($results as $result) {
    $rows[] = array(
      $result->name,
      $result->src,
      l(t('Delete'), 'admin/hosting/roles/delete/' . $result->rid),
    );
  }

  $form['roles'] = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => array(
      t('Name'), t('Source'), t('Actions'),
    ),
    '#empty' => t('No roles are available.'),
  );
  return $form;
}

function ansible_roles_settings_add() {
  $form['name'] = array(
    '#title' => t('Role Name'),
    '#type' => 'textfield',
    '#description' => t('Enter the name of the Ansible role you would like to install. May be an !link role.', array(
      l(t('Ansible Galaxy'), 'http://galaxy.ansible.com', array(
        'attributes' => array(
          'target' => '_blank',
        )
      ))
    )),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Role'),
  );

  return $form;
}

function ansible_roles_settings_add_submit(&$form, &$form_state) {

  $id = db_insert('hosting_ansible_roles_available')
    ->fields(array('name'))
    ->values(array($form_state['values']['name']))
    ->execute();

  if (!$id) {
    drupal_set_message(t('Unable to add role.'), 'error');
  }
  else {
    drupal_set_message(t('Role &name has been added.', array(
      '&name' => $form_state['values']['name'],
    )));
    drupal_goto('admin/hosting/ansible');
  }
}

function ansible_roles_settings_delete($form, $form_state, $rid) {


  // Load all available roles
  $data = db_select('hosting_ansible_roles_available', 'h')
    ->fields('h')
    ->condition('rid', $rid)
    ->execute()
    ->fetchAssoc();


  $form['note'] = array(
    '#title' => t('Role'),
    '#type' => 'item',
    '#markup' => $data['name'],
  );

  if ($data['src']) {
    $form['note'] = array(
      '#title' => t('Source'),
      '#type' => 'item',
      '#markup' => $data['src'],
    );
  }

  return confirm_form($form, t('Are you sure you want to delete this role?'), 'admin/hosting/roles');
}