<?php


/**
 * Implements drush_HOOK_pre_COMMAND()
 */
function drush_devshop_softlayer_pre_hosting_task() {

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
                $need_save = FALSE;
                drush_log('Waiting for IP addresses...', 'devshop_log');
                sleep(4);
                $server = $api->getObject();

                if (!empty($server->primaryIpAddress)) {
                    $need_save = TRUE;
                    $task->ref->ip_addresses = array(
                        $server->primaryIpAddress,
                        $server->primaryBackendIpAddress,
                    );
                }
            }

            // Save node if needed.
            if ($need_save) {
                $task->ref->no_verify = TRUE;
                if (node_save($task->ref)) {
                    drush_log('Server node saved.', 'devshop_log');
                }
                else {
                    drush_log('Server node was not saved.', 'devshop_log');
                }

                drush_log('IP address info acquired.', 'devshop_log');
                drush_log(print_r($task->ref->ip_addresses, 1), 'devshop_log');
            }

        } catch (Exception $e) {
            drush_set_error('DEVSHOP_CLOUD_ERROR', $e->getMessage());
        }


    //    $task->ref->ip_addresses =
    }
}
