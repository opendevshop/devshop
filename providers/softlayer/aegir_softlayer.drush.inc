<?php


/**
 * Implements drush_HOOK_pre_COMMAND()
 */
function drush_aegir_softlayer_pre_hosting_task() {

    $task =& drush_get_context('HOSTING_TASK');

    if ($task->ref->type == 'server') {

        // Lookup and save IP addresses.
        try {
            $service = $task->ref->services['provider'];

            // Create the Softlayer "Virtual Guest"
            $id = $service->provider_server_identifier;
            $api = $service->getClient('SoftLayer_Virtual_Guest', $id);

            // Loop until we have an IP address.
            while (empty($task->ref->ip_addresses)) {
                drush_log('Waiting for IP addresses...', 'aegir_log');
                sleep(3);
                $server = $api->getObject();

                if (!empty($server->primaryIpAddress) && !empty($server->primaryBackendIpAddress)) {
                    $need_save = TRUE;
                    $ips = array(
                        $server->primaryIpAddress,
                        $server->primaryBackendIpAddress,
                    );

                    // Save IP Addresses.
//                    drush_set_option('ip_addresses', $ips);

                    $task->ref->no_verify = TRUE;
                    if (node_save($task->ref)) {
                        drush_log('Node saved', 'success');
                    }
                    else {
                        drush_log('Node not saved', 'error');
                    }

//                    $node = node_load($task->ref->nid);
                    drush_log('IP address info acquired and saved: ' . implode(', ', $ips), 'success');
                }
            }

        } catch (Exception $e) {
            drush_set_error('AEGIR_CLOUD_ERROR', $e->getMessage());
        }


    //    $task->ref->ip_addresses =
    }
}
