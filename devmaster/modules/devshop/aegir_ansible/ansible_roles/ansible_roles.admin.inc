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
    '#required' => TRUE,
    '#description' => t('Enter the name of the Ansible role you would like to install. May be an !link role.', array(
      l(t('Ansible Galaxy'), 'http://galaxy.ansible.com', array(
        'attributes' => array(
          'target' => '_blank',
        )
      ))
    )),
  );
  $form['src'] = array(
    '#title' => t('Role Source'),
    '#type' => 'textfield',
    '#description' => t('If you wish to provide a custom role, enter the git source URL here.')
  );
  $form['version'] = array(
    '#title' => t('Role Version'),
    '#type' => 'textfield',
    '#description' => t("If you are using a git source, you may specify the version here.  Defaults to the repository's default branch.")
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add Role'),
  );

  return $form;
}

function ansible_roles_settings_add_submit(&$form, &$form_state) {

  $id = db_insert('hosting_ansible_roles_available')
    ->fields(array('name', 'src', 'version'))
    ->values(array($form_state['values']['name'], $form_state['values']['src'], $form_state['values']['version']))
    ->execute();

  if (!$id) {
    drupal_set_message(t('Unable to add role.'), 'error');
  }
  else {
    drupal_set_message(t('Role &name has been added.', array(
      '&name' => $form_state['values']['name'],
    )));

    // Queue a verify task for server master to install the roles.
    $nid = hosting_context_nid('hostmaster');
    hosting_add_task($nid, 'verify');

    drupal_goto('admin/hosting/roles');
  }
}

function ansible_roles_settings_delete($form, $form_state, $rid) {


  // Load this ansible role.
  $data = db_select('hosting_ansible_roles_available', 'h')
    ->fields('h')
    ->condition('rid', $rid)
    ->execute()
    ->fetchAssoc();

  if (empty($data)) {
    drupal_set_message(t('Role with RID &rid not found.', array('&rid' => $rid)), 'warning');
    drupal_goto('admin/hosting/roles');
  }

  $form['rid'] = array(
    '#type' => 'value',
    '#value' => $rid,
  );

  $form['name'] = array(
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

  return confirm_form($form, t('Are you sure you want to delete this role?'), 'admin/hosting/roles', t('The role will not be removed from the system but it will not be available to users.  Use the command <code>ansible-galaxy delete @name</code> to remove it from your system.', array('@name' => $data['name'])));
}

function ansible_roles_settings_delete_submit(&$form, &$form_state) {

  if (db_delete('hosting_ansible_roles_available')
    ->condition('rid', $form_state['values']['rid'])
    ->execute()) {
    drupal_set_message(t('Ansible role deleted.'));
  }
  else {
    drupal_set_message(t('Unable to delete Ansible role.'), 'error');
  }
  $form_state['redirect'] = 'admin/hosting/roles';
}