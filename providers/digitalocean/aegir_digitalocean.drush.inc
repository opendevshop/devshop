<?php


/**
 * Implements drush_HOOK_pre_COMMAND()
 */
function drush_aegir_digitalocean_pre_hosting_task() {

    $task =& drush_get_context('HOSTING_TASK');

    if ($task->task_type == 'verify' && $task->ref->type == 'server' && isset($task->ref->services['provider']) && $task->ref->services['provider']->type == 'digital_ocean') {

        // Lookup and save IP addresses.
        try {
            /** @var Provision_Service_provider_digital_ocean $service */
            $service = $task->ref->services['provider'];


            if (empty($task->ref->ip_addresses)) {

                drush_log('Acquiring IP Addresses', 'p_command');
                drush_log("Waiting for IP Address...\n", 'p_info');

              $api = $service->load_api();
              $droplet = $api->droplet();

                // Loop until we have an IP address.
                while (empty($task->ref->ip_addresses)) {
                    sleep(3);
                  $device = $droplet->getById($service->provider_server_identifier);

                  drush_log(print_r($device, 1), 'p_ok');

                    if (!empty($device->networks[0]->ipAddress)) {

                        // Save IP addresses
                        foreach ($device->networks as $network) {
                            drush_log('IP address found: ' . $network->ipAddress . "\n", 'p_ok');
                            $task->ref->new_ip_addresses[] = $network->ipAddress;
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

        } catch (Exception $e) {
            drush_set_error('AEGIR_CLOUD_ERROR', $e->getMessage());
        }
    }
}
