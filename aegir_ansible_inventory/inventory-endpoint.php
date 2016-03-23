<?php

/**
 * Returns our server inventory.
 */
function devshop_servers_inventory() {
    // Build attributes object
    $inventory = new stdClass;
    $inventory->aegir_servers->hosts = array();
    $inventory->_meta->hostvars = new stdClass();

    // Get all server nodes.
    $sql = "SELECT nid FROM node
      WHERE type = 'server' AND status = 1";
    $server_nids = db_query($sql)->fetchCol();
    $server_nodes = node_load_multiple($server_nids);

    foreach ($server_nodes as $server_node) {
        // Add host to inventory.
        $inventory->aegir_servers->hosts[] = $server_node->title;

        // Add a "group" for each individual server.
        $inventory->{$server_node->title}->hosts[] = $server_node->title;
        $inventory->{$server_node->title}->vars['ansible_user'] = 'aegir';

    }

    print json_encode($inventory, JSON_UNESCAPED_SLASHES);
    exit;
}