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

            // Load DNS from digitalocean API on every verify.
            drush_log('Preparing DNS...', 'p_command');
            drush_log("Waiting for IP Address from DigitalOcean API...\n", 'p_info');
            $public_ip_address = null;
            $new_ips = [];

            // Loop until we have an IP address.
            while (empty($new_ips)) {
                sleep(3);
                $device = $droplet->getById($service->provider_server_identifier);

                if (!empty($device->networks[0]->ipAddress)) {

                    // Save IP addresses
                    foreach ($device->networks as $network) {
                        drush_log('IP address found: ' . $network->ipAddress . "\n", 'p_ok');
                        $new_ips[] = $network->ipAddress;

                        // Save public IP address for DNS Creation.
                        if (empty($public_ip_address) && $network->type == 'public') {
                          $public_ip_address = $network->ipAddress;
                        }
                    }

                    if ($public_ip_address) {
                      array_unshift($new_ips, $public_ip_address);
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
                  if ($record->data != $public_ip_address) {

                  }
                  $record_found = TRUE;
                }
              }

              // Create domain record.
              if (!$record_found) {
                $created = [];
                $created[] = $domainRecord->create($base_domain, 'A', $subdomain, $public_ip_address, 0);
                $created[] = $domainRecord->create($base_domain, 'A', '*.' . $subdomain, $public_ip_address, 0);
                drush_log(dt('Domain Records created successfully!'), 'p_ok');
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
            drush_log('Waiting for DNS for ' . $service->getServer()->title, 'p_command');
            $completed = FALSE;

            while (!$completed) {
              sleep(3);
              $hostname = gethostbyaddr($public_ip_address);
              // If gethostbyaddr returns the hostname, we are done.
              drush_log("gethostbyaddr($public_ip_address): $hostname \n", 'p_info');

              if ($hostname == $task->ref->title) {
                $completed = TRUE;
              }
            }
            drush_log('DNS record active!', 'p_ok');

            // Loop until SSH is available.
            $ssh_string = "root@{$task->ref->title}";
            $completed = FALSE;
            drush_log('Waiting for SSH to ' . $ssh_string, 'p_command');
            while (!$completed) {
              sleep(1);
              exec("ssh -q {$ssh_string} exit", $out, $code);
              $output = implode(PHP_EOL, $out);
              if ($code == 0) {
                $completed = TRUE;
              }
              elseif (strpos($output, 'Permission denied (publickey)')) {
                drush_log(dt('SSH failed: %output', array(
                  '%command' => $ssh_string,
                  '%output' => $output,
                )), 'warning');
                $completed = TRUE;
              }
            }
            drush_log(dt('SSH connection active!'), 'p_ok');

        } catch (Exception $e) {
            drush_set_error('AEGIR_CLOUD_ERROR', $e->getMessage());
        }  catch (\DigitalOceanV2\Exception\HttpException $e) {
          drupal_set_message($base_domain);
        }

    }
}
