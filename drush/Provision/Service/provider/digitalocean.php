<?php

use DigitalOceanV2\Adapter\BuzzAdapter;
use DigitalOceanV2\DigitalOceanV2;

class Provision_Service_provider_digital_ocean extends Provision_Service_provider {

  public $provider = 'digital_ocean';


  function verify_server_cmd() {

    $digitalocean = $this->load_api();
    $droplet = $digitalocean->droplet();
    $cloud = $droplet->getById($this->server->provider_server_identifier);
    if ($cloud->status == 'active') {

      $ips = array();
      foreach ($cloud->networks as $network) {
        $ips[] = $network->ipAddress;
      }

      drush_set_option('ip_addresses', $ips);
      drush_log('[DEVSHOP] Cloud Server IPs updated.', 'ok');
    }
    else {
      drush_set_error('DEVSHOP_CLOUD_SERVER_NOT_ACTIVE', dt('The remote cloud server is not in an active state.'));
    }
  }


  /**
   * This method is called once per server verify, triggered from hosting task.
   *
   * $this->server: Provision_Context_server
   */
  function save_server() {
    // Look for provider_server_identifier
    $server_identifier = $this->server->provider_server_identifier;
    // If server ID is already found, move on.
    if (!empty($server_identifier)) {
      drush_log('[DEVSHOP] Server Identifier Found: ' . $server_identifier . '  Not creating new server.', 'ok');

      $user_data = $this->default_user_data();
      drush_log(print_r($user_data, TRUE));
    }
    // If there is no server ID, create the server.
    else {
      drush_log('[DEVSHOP] Server Identifier not found.  Creating new server!', 'ok');

      $options = $this->server->provider_options;
      $digitalocean = $this->load_api();
      $droplet = $digitalocean->droplet();

      //$user_data = !empty($options['user_data']) ? $options['user_data'] :  $this->default_user_data();

      if ($options['remote_server']) {
        $user_data = $this->default_user_data();
	drush_log(print_r($user_data, TRUE));
      }

      $keys = array_filter($options['keys']);
      $keys = array_values($keys);

      $created = $droplet->create($this->server->remote_host, $options['region'], $options['size'], $options['image'],
        $options['backups'], $options['ipv6'], $options['private_networking'], $keys, $user_data);

      $this->server->setProperty('provider_server_identifier', $created->id);
      drush_log("[DEVSHOP] Server Identifier found: $created->id. Assumed server was created.", 'ok');
    }
  }

  function load_api(){
    $token = drush_get_option('digital_ocean_token');
    require_once dirname(__FILE__) . '/digital-ocean-master/vendor/autoload.php';
    require_once dirname(__FILE__) . '/digital-ocean-master/src/DigitalOceanV2.php';

    $adapter = new BuzzAdapter($token);
    $digitalocean = new DigitalOceanV2($adapter);
    return $digitalocean;
  }


  function default_user_data() {

    if (isset($this->server->http_service_type)) {
      $http = $this->server->http_service_type;
      switch ($http) {
      case 'apache':
        $commands = <<<EOT
ln -s /var/aegir/config/apache.conf /etc/apache2/conf.d/aegir.conf
a2enmod rewrite
EOT;
      	break;
      case 'nginx':
      case 'nginx_ssl':
        $commands = "ln -s /var/aegir/config/nginx.conf /etc/nginx/conf.d/aegir.conf";
        break;
      default:
        $commands = '';
        break;
      }
    }

    $mysql_command = '';
    if (isset($this->server->db_service_type)) {
      $db = $this->server->db_service_type;
      if ($db == 'mysql') {
        $creds = array_map('urldecode', parse_url($this->server->master_db));
        $password = $creds['pass'];
        $aegir_host = provision_fqdn();
        $mysql_command = "mysql -u root -p$(cat /etc/motd.tail | awk -F'password is ' '{print $2}' | xargs) -e 'GRANT ALL PRIVILEGES ON *.* TO root@$aegir_host IDENTIFIED BY \"$password\" WITH GRANT OPTION;FLUSH PRIVILEGES;'";
      }
    }

    $handle = fopen('/var/aegir/.ssh/id_rsa.pub', 'r');
    $ssh_key = fgets($handle);
    fclose($handle);

    $config = <<<EOT
#!/bin/bash
adduser --system --group --home /var/aegir aegir
adduser aegir www-data
$commands
$mysql_command
touch /etc/sudoers.d/aegir
cat 'aegir ALL=NOPASSWD: ALL' > /etc/sudoers.d/aegir
mkdir /var/aegir/.ssh
touch /var/aegir/.ssh/authorized_keys
cat $ssh_key >> /var/aegir/.ssh/authorized_keys
chmod -R 400 /var/aegir/.ssh
EOT;

    return $config;
  }


}
