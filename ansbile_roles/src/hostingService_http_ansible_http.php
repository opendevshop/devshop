<?php
/**
 * @file
 * Hosting service classes for the Hosting web server module.osting service classes for the Hosting web server module.
 */



class hostingService_http_ansible_http extends hostingService_http_public {
    public $type = 'apache';

    protected $has_restart_cmd = TRUE;

    function default_restart_cmd() {
        $command = '/usr/sbin/apache2ctl'; # a proper default for most of the world
        foreach (explode(':', $_SERVER['PATH']) as $path) {
            $options[] = "$path/apache2ctl";
            $options[] = "$path/apachectl";
        }
        # try to detect the apache restart command
        $options[] = '/usr/local/sbin/apachectl'; # freebsd
        $options[] = '/usr/sbin/apache2ctl'; # debian + apache2
        $options[] = $command;

        foreach ($options as $test) {
            if (is_executable($test)) {
                $command = $test;
                break;
            }
        }

        return "sudo $command graceful";
    }
}
