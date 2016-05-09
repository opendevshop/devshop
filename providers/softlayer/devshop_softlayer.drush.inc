<?php

module_load_include('service.inc', 'devshop_cloud');
require_once dirname(__FILE__) . '/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

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
            $server = $api->getObject();
            $task->ref->ip_addresses = array(
                $server->primaryIpAddress,
                $server->primaryBackendIpAddress,
            );

            $task->ref->no_verify = TRUE;
            node_save($task->ref);

            drush_log('IP address info acquired.', 'devshop_info');

        } catch (Exception $e) {
            drush_set_error('DEVSHOP_CLOUD_ERROR', $e->getMessage());
        }


    //    $task->ref->ip_addresses =
    }
}
