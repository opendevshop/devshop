<?php

/**
 * Prints the inventory object out as json.
 *
 * @TODO: Access control!
 */
function aegir_ansible_inventory_endpoint() {

    $inventory = aegir_ansible_inventory_data();
    print json_encode($inventory, JSON_UNESCAPED_SLASHES);
    exit;
}

/**
 * Returns an "inventory" object from our hostmaster.
 * @return stdClass
 */
function aegir_ansible_inventory_data() {
    // Build attributes object
    $inventory = new stdClass;
    $inventory->aegir_servers->hosts = array();
    $inventory->_meta->hostvars = new stdClass();

    // Get all server nodes.
    $sql = "SELECT n.nid FROM node n
      LEFT JOIN hosting_server s ON n.nid = s.nid
      WHERE type = 'server' AND n.status = :node_status AND 
        (s.status = :hosting_server_enabled OR s.status = :hosting_server_queued)";
    $server_nids = db_query($sql, array(
      ':node_status' => 1,
      ':hosting_server_enabled' => HOSTING_SERVER_ENABLED,
      ':hosting_server_queued' => HOSTING_SERVER_QUEUED,
    ))->fetchCol();
    $server_nodes = node_load_multiple($server_nids);

    foreach ($server_nodes as $server_node) {

        // If server has no roles we assume it shouldn't be in the inventory.
        if (!isset($server_node->roles) || empty($server_node->roles)) {
          continue;
        }

        // Add host to inventory.
        $inventory->aegir_servers->hosts[] = $server_node->title;

        // Server Variables
        // These variables are applied just to that host.
        $inventory->{$server_node->title}->hosts[] = $server_node->title;

        if (!empty($server_node->ansible_vars)) {
            $inventory->{$server_node->title}->vars = $server_node->ansible_vars;
        }
        else {
          $inventory->{$server_node->title}->vars = new stdClass();
        }

        // The variable 'ansible_user' maybe used to force ansible to connect via this user.
        // This is disabled so that our ansible runner can connect as root via the command line.
        // If this variable is set, the `-u root` command line option is ignored.
        // $inventory->{$server_node->title}->vars['ansible_user'] = 'aegir';

        // Load another group based on the server's hosting_name (hosting context).
        if (isset($server_node->hosting_name) && isset($server_node->title) && isset($inventory->{$server_node->title})) {
            $inventory->{$server_node->hosting_name} = $inventory->{$server_node->title};
        }

        // Add a "group" for each service type.
        foreach ($server_node->services as $service => $service_data) {

          // Add to "service" group ("http", "db")
          $inventory->{$service}->hosts[] = $server_node->title;

          // Add to "service type" group ("apache", "nginx", "mysql")
          $inventory->{$service_data->type}->hosts[] = $server_node->title;
        }
    }

    // Allow modules to alter inventory.
    drupal_alter('ansible_inventory', $inventory);

    return $inventory;
}