
# DEVSHOP VARNISH CONFIG FILE

<?php foreach ($server->ip_addresses as $ip) : ?>
    NameVirtualHost <?php print $ip . ":" . $http_port . "\n"; ?>
<?php endforeach; ?>

<IfModule !ssl_module>
    LoadModule ssl_module modules/mod_ssl.so
</IfModule>

<?php include(provision_class_directory('Provision_Config_Apache_Server') . '/server.tpl.php'); ?>
