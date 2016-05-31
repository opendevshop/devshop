<?php


/**
 * Implements drush_HOOK_pre_COMMAND()
 */
function drush_aegir_softlayer_pre_hosting_task() {

    $task =& drush_get_context('HOSTING_TASK');

    if ($task->ref->type == 'server' && isset($task->ref->services['provider']) && $task->ref->services['provider']->type == 'softlayer') {

        // Lookup and save IP addresses.
        try {
            $service = $task->ref->services['provider'];

            // Create the Softlayer "Virtual Guest"
            $id = $service->provider_server_identifier;
            $api = $service->getClient('SoftLayer_Virtual_Guest', $id);

            if (empty($task->ref->ip_addresses)) {

            drush_log('Acquiring IP Addresses', 'devshop_command');
            drush_log("Waiting for IP Address...\n", 'devshop_info');

            // Loop until we have an IP address.
            while (empty($task->ref->ip_addresses)) {
                sleep(3);
                $server = $api->getObject();

                if (!empty($server->primaryIpAddress) && !empty($server->primaryBackendIpAddress)) {
                    $ips = array(
                        $server->primaryIpAddress,
                        $server->primaryBackendIpAddress
                    );
                    $need_save = TRUE;
                    drush_log('IP address loaded from API: ' . implode(', ', $ips) . "\n", 'devshop_info');

                    // Save IP addresses
                    $task->ref->new_ip_addresses = $ips;
                    $task->ref->no_verify = TRUE;

                    _hosting_ip_save($task->ref, 'update');
                    $node = node_load($task->ref->nid);
                    if (empty($node->ip_addresses)) {
//                        drush_log('Unable to save IP Addresses.', 'devshop_error');
                        return drush_set_error('AEGIR_CLOUD_ERROR', 'IP addresses were not saved!');
                    }
                    else {
                        drush_log('IP address from node: ' . implode(', ', $node->ip_addresses), 'devshop_ok');
                    }
                    return;
                }
            }
            }

        } catch (Exception $e) {
            drush_set_error('AEGIR_CLOUD_ERROR', $e->getMessage());
        }


    //    $task->ref->ip_addresses =
    }
}
