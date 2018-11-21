<?php


/**
 * Implements drush_HOOK_pre_COMMAND()
 */
function drush_aegir_digitalocean_pre_hosting_task() {

    $task =& drush_get_context('HOSTING_TASK');

    if ($task->task_type == 'verify' && $task->ref->type == 'server' && isset($task->ref->services['provider']) && $task->ref->services['provider']->type == 'digital_ocean') {

        // Lookup and save IP addresses.
        try {
            /** @var hostingService_provider_digital_ocean $service */
            $service = $task->ref->services['provider'];

            $api = $service->load_api();
            $droplet = $api->droplet();

            if (empty($task->ref->ip_addresses)) {

                drush_log('Acquiring IP Addresses', 'p_command');
                drush_log("Waiting for IP Address...\n", 'p_info');


                // Loop until we have an IP address.
                while (empty($task->ref->ip_addresses)) {
                    sleep(3);
                    $device = $droplet->getById($service->provider_server_identifier);

                    if (!empty($device->networks[0]->ipAddress)) {
                        $first_ip = $device->networks[0]->ipAddress;

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

            // Save a DNS record
            if (variable_get('aegir_cloud_digital_ocean_create_dns', FALSE)) {
              $url_parts = explode('.', $service->getServer()->title);
              $base_domain = implode('.', array_slice($url_parts, -2, 2));
              $subdomain = str_replace('.' . $base_domain, '', $service->getServer()->title);

              $domainRecord = $api->domainRecord();
              $record_found = FALSE;

              $domainRecords = $domainRecord->getAll($base_domain);
              foreach ($domainRecords as $record) {
                if ($record->name == $subdomain) {

                  // @TODO: If IP address changed, Update Record?
                  if ($record->data != $first_ip) {

                  }
                  $record_found = TRUE;
                }
              }

              // Create domain record.
              if (!$record_found) {
                $created = $domainRecord->create($base_domain, 'A', $subdomain, current($task->ref->ip_addresses), 0);

                //@TODO! This is devshop.cloud specific! Remove it!
                $created = $domainRecord->create($base_domain, 'A', '*.' . $subdomain, current($task->ref->ip_addresses), 0);
                drush_log(dt('Domain Record created successfully!'), 'p_ok');
              }
            }

            // Loop until we have completed all provisioning steps.
            drush_log('Waiting for Server' . $service->getServer()->title, 'p_command');
            $completed = FALSE;
            while (!$completed) {
              sleep(3);
              $device = $droplet->getById($service->provider_server_identifier);
              drush_log("Droplet status: $device->status \n", 'p_info');

              // Save provider data
              $task->ref->no_verify = TRUE;
              $task->ref->services['provider']->provider_data = $device;
              node_save($task->ref);

              if ($device->status == 'active') {
                drush_log('Device active!', 'p_ok');
                $completed = TRUE;
              }
            }

            // Loop until DNS resolves.
            drush_log('Waiting for DNS for ... ' . $service->getServer()->title, 'p_command');
            $completed = FALSE;
            while (!$completed) {
              sleep(3);

              $records = dns_get_record($service->getServer()->title);
              foreach ($records as $record) {
                if (
                  ($record['type'] == 'A' || $record['type'] == 'CNAME') &&
                  $record['ip'] == current($task->ref->ip_addresses)) {

                  drush_log("DNS Record Found! " . print_r($record, 1), 'p_ok');
                  $completed = TRUE;
                }
              }
            }

        } catch (Exception $e) {
            drush_set_error('AEGIR_CLOUD_ERROR', $e->getMessage());
        }  catch (\DigitalOceanV2\Exception\HttpException $e) {
          drupal_set_message($base_domain);
        }

    }
}
