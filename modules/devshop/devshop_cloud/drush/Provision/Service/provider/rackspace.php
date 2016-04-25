<?php
//
//define('DRUSH_DEVUDO_ERROR', 10);
//
//
///**
// * Rackspace provider.
// */
//class Provision_Service_provider_rackspace extends Provision_Service_provider {
//
//  /**
//   * This is run immediately after provision loads "servers driver for devudo service"
//   *
//   * Also it runs many times, for some reason.
//   */
//  function init_server() {
//    parent::init_server();
//
//    $this->server->rackspace_id = drush_get_option('rackspace_id', '');
//    $this->server->rackspace_image = drush_get_option('rackspace_image', '');
//    $this->server->rackspace_flavor = drush_get_option('rackspace_flavor', '');
//    $this->server->attributes_json = drush_get_option('attributes_json', '');
//    $this->server->role = drush_get_option('role', '');
//
//  }
//
//  /**
//   * This is run immediately after provision saves the server config files.
//   *
//   *
//   *   Provision client home path /var/aegir/clients is writable.
//   *   [DEVUDO] Verifying Server anotherfaker
//   */
//  function verify_server_cmd() {
//    drush_log('[DEVUDO] Verifying Server ' . d()->remote_host, 'ok');
//
//    $server_fqdn = d()->remote_host;
//    $role = $this->server->role;
//    $rackspace_flavor = $this->server->rackspace_flavor;  // 2
//    $rackspace_image = $this->server->rackspace_image;
//    $rackspace_id = $this->server->rackspace_id;
//    $attributes = $this->server->attributes_json;
//
//    $ips = array();
//
//    // Look for this chef node on Chef Server
//    drush_log("[DEVUDO] Looking for chef node $server_fqdn on chef server", 'ok');
//    $chef_node = shop_get_server($server_fqdn);
//
//    // If shop_get_server() returns a string, knife node show didn't work.
//    if (is_string($chef_node)) {
//
//      // If the error is NOT object not found, there was a more serious error
//      if (strpos($chef_node, 'ERROR: The object you are looking for could not be found') !== 0){
//        return drush_set_error(DRUSH_DEVUDO_ERROR, '[DEVUDO] knife failed: ' . $chef_node);
//      }
//      // Otherwise, we just don't have a chef node of that name yet.
//      // So, create a new server.
//      drush_log("[DEVUDO] Chef Node not found. Creating server...", 'ok');
//
//      drush_log("[DEVUDO] Running: drush server-create $server_fqdn --role=$role --rackspace_flavor=$rackspace_flavor --rackspace_image=$rackspace_image --attributes=$attributes", 'ok');
//
//      drush_set_option('rackspace_flavor', $rackspace_flavor);
//      drush_set_option('rackspace_image', $rackspace_image);
//      drush_set_option('role', $role);
//      drush_set_option('attributes', $attributes);
//
//      $data = drush_shop_provision_server_create($server_fqdn);
//      $ips[] = $data['Public IP Address'];
//      $ips[] = $data['Private IP Address'];
//
//      // Save for shop_hosting_post_hosting_verify_task()
//      drush_set_option('rackspace_id', $data['Instance ID']);
//      drush_set_option('ip_addresses', $ips);
//    }
//    // If we got a server node... run chef-client to update it.
//    else {
//      $ip = $chef_node->automatic->ipaddress;
//      drush_log("[DEVUDO] Chef node found with name:$server_fqdn ip:$ip Preparing attributes...", 'ok');
//
//      // @TODO: Copy the attributes file and run chef-client again.
//      // Save new json data to file
//
//      $json_path = "/tmp/$server_fqdn.json";
//      $attributes_json = $attributes;
//      file_put_contents($json_path, $attributes_json);
//
//      // Sync file to server
//      // Use IP in case something is wrong with DNS
//      if (!empty($ip)){
//        $host = $ip;
//      }
//      else {
//        $host = $server_fqdn;
//      }
//
//      // @TODO: This line implies that aegir already has ssh access to devudo@host
//      shop_exec("scp $json_path devudo@$host:~/attributes.json");
//
//      // Run chef-client to update the server itself.
//      $chef_client_cmd = "sudo /usr/bin/chef-client -j attributes.json";
//      $chef_client_cmd_exec = escapeshellarg($chef_client_cmd);
//      drush_log("[DEVUDO] Running chef-client on $server_fqdn:", 'ok');
//      shop_exec("knife ssh name:$server_fqdn -x devudo $chef_client_cmd_exec -a ipaddress");
//    }
//    parent::verify_server_cmd();
//  }
//
//
//
//  function config_data($config = null, $class = null) {
//    $data = parent::config_data($config, $class);
//    $data['rackspace_id'] = $this->server->rackspace_id;
//    $data['rackspace_image'] = $this->server->rackspace_image;
//    $data['rackspace_flavor'] = $this->server->rackspace_flavor;
//    $data['role'] = $this->server->role;
//    $data['attributes_json'] = $this->server->attributes_json;
//    return $data;
//  }
//
//  static function option_documentation() {
//    return array(
//      '--rackspace_id' => 'The unique rackspace server ID.',
//      '--rackspace_image' => 'The rackspace server image.',
//      '--rackspace_flavor' => 'The rackspace server flavor.',
//      '--role' => 'The chef role.',
//      '--attributes_json' => 'JSON encoded attributes.',
//    );
//  }
//
//  /**
//   * Ask the web server to check for and load configuration changes.
//   */
//  function parse_configs() {
//    return TRUE;
//  }
//}
