<?php
/**
 * @file
 * Provide the hosting serivce classes for database integration.
 */

class hostingService_ansible_roles extends hostingService {
  public $service = 'ansible_roles';

  /**
   * Make the server node vailable in the service.
   * 
   * hostingService_ansible_roles constructor.
   * @param $node
   * @param null $values
   */
  function __construct($node, $values = NULL) {
    parent::__construct($node, $values);
    $this->server->node = $node;
  }

  function getRoles() {
    return array();
  }

  /**
   * Check if a server has a certain role.
   *
   * @param $name
   * @return bool
   */
  function hasRole($name) {
    return in_array($name, $this->roles);
  }

  /**
   * The list of ansible roles that this service depends on.
   *
   * @return array
   */
  function getRoleNames() {
    $roles = $this->getRoles();
    foreach ($roles as $role) {
      if (isset($role['name'])) {
        $names[] = $role['name'];
      }
      else {
        $names[] = $role;
      }
    }
    return $names;
  }
}

/**
 * Custom Ansible Roles service.
 */
class hostingService_ansible_roles_custom extends hostingService_ansible_roles {
  public $type = 'custom';
  public $name = 'Custom Roles';

  public $has_port = FALSE;
  public $ansible_vars = array();

  function form(&$form) {
    parent::form($form);
    $form['roles'] = array(
      '#title' => t('Choose the roles for this server.'),
      '#type' => 'checkboxes',
      '#options' => $this->getRolesOptions(),
      '#default_value' => isset($this->roles) ? $this->roles : array(),
    );

    if (empty($form['roles']['#options'])) {
      $form['roles'] = array(
        '#markup' => t('No roles available.')
      );

      if (user_access('administer ansible roles')) {
        $form['link'] = array(
            '#markup' => l(t('Add roles'), 'admin/hosting/roles'),
        );
      }
    }
  }

  function insert()
  {
      parent::insert();
      foreach ($this->roles as $role) {
        if ($role) {
          db_insert('hosting_ansible_roles')
            ->fields(array(
              'vid' => $this->server->vid,
              'nid' => $this->server->nid,
              'role' => $role,
            ))
            ->execute();
        }
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

  public function view(&$render)
  {
    $render['title'] = '';
  }

  /**
   * The list of ansible roles that this service depends on.
   *
   * @return array
   */
  function getRoles() {

    // Load all available roles
    $results = db_select('hosting_ansible_roles_available', 'h')
        ->fields('h')
        ->execute()
        ->fetchAllAssoc('name');

    foreach ($results as $result) {
      $options[$result->name] = $result;
    }

    return $options;
  }

  /**
   * The list of available ad hoc roles, ready for a form array.
   *
   * @return array
   */
  function getRolesOptions() {
    $roles = $this->getRoles();
    foreach ($roles as $role) {
      $options[$role->name] = $role->name;
    }
    return $options;
  }

  /**
   * The list of available role names, in a simple array.
   *
   * @return array
   */
  function getRoleNames() {
    $roles = $this->getRoles();
    foreach ($roles as $role) {
      $names[] = $role->name;
    }
    return $names;
  }
}
