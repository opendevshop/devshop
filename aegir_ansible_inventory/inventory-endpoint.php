<?php

/**
 * Returns our server inventory.
 */
function devshop_servers_inventory() {
    // Build attributes object
    $inventory = new stdClass;
    $inventory->devmasters->hosts = array();
    $inventory->_meta->hostvars = new stdClass();

    // DevShop System Administrators.
    // Add any users with the role "system administrator"
    $sql = "SELECT u.uid FROM users u
      INNER JOIN users_roles ur ON u.uid = ur.uid
      INNER JOIN role r ON ur.rid = r.rid
      WHERE r.name = :role AND u.status = 1";
    $admin_uids = db_query($sql, array(':role' => 'system administrator'))->fetchCol();

    // Get all users and their keys.
    $sql = "SELECT u.uid FROM users u
      WHERE u.status = 1";
    $uids = db_query($sql)->fetchCol();
    $users = user_load_multiple($uids);

    // Add system administrators
    foreach ($admin_uids as $uid) {
        $account = $users[$uid];

        $inventory->devmasters->vars->system_administrators[$account->name] =  array(
            'username' => $account->name,
            'authorized_keys' => url("keys/$account->name", array('absolute' => TRUE)),
        );
    }

    // Get all server nodes.
    $sql = "SELECT nid FROM node
      WHERE type = 'server' AND status = 1";
    $server_nids = db_query($sql, array(':role' => 'system administrator'))->fetchCol();
    $server_nodes = node_load_multiple($server_nids);

    foreach ($server_nodes as $server_node) {
        // Add host to inventory.
        $inventory->devmasters->hosts[] = $server_node->title;

        // Add server users
        $server_users = field_get_items('node', $server_node, 'devshop_server_users');
        if (!empty($server_users)) {
            foreach ($server_users as $data) {
                $account = $users[$data['target_id']];
                $inventory->_meta->hostvars->{$server_node->title}->users[$account->name] = array(
                    'username' => $account->name,
                );
            }
        }
    }

    print json_encode($inventory, JSON_UNESCAPED_SLASHES);
    exit;
}