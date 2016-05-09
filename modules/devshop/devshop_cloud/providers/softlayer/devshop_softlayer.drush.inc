<?php

module_load_include('service.inc', 'devshop_cloud');
require_once dirname(__FILE__) . '/softlayer-api-php-client/SoftLayer/SoapClient.class.php';

/**
 * Implementation of hook_post_hosting_TASK_TYPE_task() for verified tasks.
 */
function devshop_softlayer_post_hosting_verify_task($task, $data) {

    if ($task->ref->type == 'server') {

        // Lookup and save IP addresses.
        try {
            $service = $task->ref->services['provider'];

            // Create the Softlayer "Virtual Guest"
            $id = $service->provider_server_identifier;
            $api = $service->softlayer_client('SoftLayer_Virtual_Guest', $id);
            $server = $api->getObject();

            drush_log(print_r($server, 1), 'devshop_log');

        } catch (Exception $e) {
            drush_set_error('DEVSHOP_CLOUD_ERROR', $e->getMessage());
        }


    //    $task->ref->ip_addresses =
    }
}
