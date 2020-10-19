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
            $servername = $service->getServer()->title;

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
                            $new_ips[] = $network->ipAddress;
                        }
                        $task->ref->ip_addresses = $new_ips;
                        $task->ref->no_verify = TRUE;
                        node_save($task->ref);

                        // reload the node to check that IPs were saved.
//                        $server = node_load($task->ref->nid, TRUE);
//                        $task->ref = $server;
//                        if (empty($task->ref->ip_addresses)) {
//                            drush_log('IP addresses were not found when reloading the node... not sure why.', 'warning');
//                          $task->ref->ip_addresses = $new_ips;
//                        }
                        break;
                    }
                }
            }
            else {
              $first_ip = current($task->ref->ip_addresses);
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
                $created = $domainRecord->create($base_domain, 'A', $subdomain, $first_ip, 0);

                //@TODO! This is devshop.cloud specific! Remove it!
                $created = $domainRecord->create($base_domain, 'A', '*.' . $subdomain, $first_ip, 0);
                drush_log(dt('Domain Record created successfully!'), 'p_ok');
              }
            }

            // Loop until we have completed all provisioning steps.
            drush_log(dt('Waiting for droplet %droplet to be ready.', array(
              '%droplet' => $service->getServer()->title,
            )), 'p_command');
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
              $hostname = gethostbyaddr($first_ip);
              // If gethostbyaddr returns the hostname, we are done.
              if ($hostname == $task->ref->title) {
                $completed = TRUE;
              }
            }

            // Loop until SSH is available.
            drush_log('Waiting for SSH ... ', 'p_command');
            $completed = FALSE;
            while (!$completed) {
              sleep(1);
              exec("ssh -q root@{$task->ref->title} exit", $out, $code);
              if ($code == 0) {
                $completed = TRUE;
              }
            }

        } catch (Exception $e) {
            drush_set_error('AEGIR_CLOUD_ERROR', $e->getMessage());
        }  catch (\DigitalOceanV2\Exception\HttpException $e) {
          drupal_set_message($base_domain);
        }

    }
}
