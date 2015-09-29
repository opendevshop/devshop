<?php

/**
 * Apache Varnish service class.
 */
class Provision_Service_http_apache_varnish extends Provision_Service_http_apache {

    protected $application_name = 'apache_varnish';
    protected $has_restart_cmd = TRUE;

    function default_restart_cmd() {
        // The apache service defines it's restart command as a static
        // method so that we can make use of it here.
        return Provision_Service_http_apache::apache_restart_cmd();
    }

    function cloaked_db_creds() {
        return TRUE;
    }

    /**
     * Initialize the configuration files.
     *
     * These config classes are a mix of the SSL and Non-SSL apache
     * classes. In some cases they extend the Apache classes too.
     */
    function init_server() {
        parent::init_server();

        // Replace the server config with our own. See the class for more info.
        $this->configs['server'][] = 'Provision_Config_ApacheVarnish_Apache_Server';
        $this->configs['server'][] = 'Provision_Config_ApacheVarnish_Varnish_Server';
        $this->configs['site'][] = 'Provision_Config_Apache_Varnish_Site';
    }

    /**
     * Restart apache to pick up the new config files.
     */
    function parse_configs() {
        return $this->restart();
    }
}
