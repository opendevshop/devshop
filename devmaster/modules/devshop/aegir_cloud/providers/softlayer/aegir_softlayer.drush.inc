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

            drush_log('Acquiring IP Addresses', 'p_command');
            drush_log("Waiting for IP Address...\n", 'p_info');

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
                    drush_log('IP address loaded from API: ' . implode(', ', $ips) . "\n", 'p_ok');

                    // Save IP addresses
                    $task->ref->ip_addresses = $ips;
                    $task->ref->no_verify = TRUE;
                    if (!node_save($task->ref)) {
                        return drush_set_error('AEGIR_CLOUD_ERROR', 'IP addresses were not saved!');
                    }
                    else {
                        drush_log('IP address from node: ' . implode(', ', $task->ref->ip_addresses), 'p_ok');
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
