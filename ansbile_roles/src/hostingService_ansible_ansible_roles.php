<?php
/**
 * @file
 * Provide the hosting serivce classes for database integration.
 */

class hostingService_ansible extends hostingService {
  public $service = 'ansible';
}

/**
 * A MySQL specific db service implemenstation class.
 */
class hostingService_ansible_ansible_roles extends hostingService_ansible {
  public $type = 'ansible_roles';
  public $has_port = FALSE;
  public $ansible_vars = array();

  function form(&$form) {
    parent::form($form);
    $form['roles'] = array(
      '#title' => t('Choose the roles for this server.'),
      '#type' => 'checkboxes',
      '#options' => $this->getAvailableRoles(),
      '#default_value' => isset($this->roles)? $this->roles: array(),
    );
  }

  function insert()
  {
      parent::insert();
      foreach ($this->roles as $role) {
        db_insert('hosting_ansible_roles')
          ->fields(array(
            'vid' => $this->server->vid,
            'nid' => $this->server->nid,
            'role' => $role,
          ))
          ->execute();
      }
  }

  function update() {
      parent::update();

      // Delete the listed roles and insert again.
      $this->delete();
      $this->insert();
  }

  function delete_revision() {
    parent::delete_revision();
    db_delete('hosting_ansible_roles')
      ->condition('vid', $this->server->vid)
      ->execute();
  }

  function delete() {
    parent::delete();
    db_delete('hosting_ansible_roles')
      ->condition('nid', $this->server->nid)
      ->execute();
  }

  function load() {
    parent::load();
    $this->roles = db_select('hosting_ansible_roles', 'a')
      ->fields('a', array('role'))
      ->condition('nid', $this->server->nid)
      ->condition('vid', $this->server->vid)
      ->execute()
      ->fetchCol()
    ;
  }

  /**
   * The list of ansible roles that this service depends on.
   *
   * @return array
   */
  function getAvailableRoles() {
    return array(
      'geerlingguy.security' => 'geerlingguy.security',
      'aegir.devmaster' => 'aegir.devmaster',
    );
  }
}
