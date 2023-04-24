<?php


/**
 * Implements drush_HOOK_pre_COMMAND()
 */
function drush_aegir_cloud_packet_pre_hosting_task() {

    $task =& drush_get_context('HOSTING_TASK');

    if ($task->task_type == 'verify' && $task->ref->type == 'server' && isset($task->ref->services['provider']) && $task->ref->services['provider']->type == 'packet') {

        // Lookup and save IP addresses.
        try {
            $service = $task->ref->services['provider'];


            if (empty($task->ref->ip_addresses)) {

                drush_log('Acquiring IP Addresses', 'p_command');
                drush_log("Waiting for IP Address...\n", 'p_command_log');

                // Loop until we have an IP address.
                while (empty($task->ref->ip_addresses)) {
                    sleep(3);
                    $device = $service->getApi()->device()->get($service->provider_options['project'], $service->provider_server_identifier);

//                    drush_log('Device:', 'p_ok');
//                    drush_log(print_r($device, 1), 'p_ok');

                    if (!empty($device->ipAddresses)) {

                        // Save IP addresses
                        foreach ($device->ipAddresses as $ip) {
                            drush_log('IP address found: ' . $ip->address . "\n", 'p_ok');
                            $task->ref->ip_addresses[] = $ip->address;
                        }
                        $task->ref->no_verify = TRUE;
                        node_save($task->ref);

                        // reload the node to check that IPs were saved.
                        $server = $task->ref = node_load($task->ref->nid);
                        if (empty($server->ip_addresses)) {
                            return drush_set_error('AEGIR_CLOUD_ERROR', 'IP addresses were not saved!');
                        }
                        break;
                    }
                }
            }

            // Loop until we have completed all provisioning steps.
            drush_log('Waiting for Server', 'p_command');
            $completed = FALSE;
            $completed_events = [];
            while (!$completed) {
                sleep(3);
                $device = $service->getApi()->device()->get($service->provider_options['project'], $service->provider_server_identifier);
                if (empty($device)) {
                    return drush_set_error('AEGIR_CLOUD_ERROR', dt('Unable to load device information.'));
                }

                // Log if an event completed.
                if (isset($device->provisioningEvents )) {
                    foreach ($device->provisioningEvents as $event) {
                        if (!empty($event->id) && empty($completed_events[$event->type])) {
                            drush_log((int) $device->provisioningPercentage . "% | " . $event->body . "\n", 'p_command_log');

                            // Save provider data
                            $task->ref->no_verify = TRUE;
                            $task->ref->services['provider']->provider_data = $device;
                            node_save($task->ref);

                            // Save completed events to an array so we don't log again.
                            $completed_events[$event->type] = $event;
                            break;
                        }
                    }
                }

                // When device is active, Stop holding up verification process.
                if ($device->state == 'active') {
                    drush_log('All events completed!', 'p_ok');
                    return;
                }
            }


        } catch (Exception $e) {
            drush_set_error('AEGIR_CLOUD_ERROR', $e->getMessage());
        }


    //    $task->ref->ip_addresses =
    }
}
