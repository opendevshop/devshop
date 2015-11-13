# Varnish Defaults
START=yes
NFILES-131072
MEMLOCK=8200

# Varnish Defaults + Custom Port
DAEMON_OPTS="-a :<?php print $http_varnish_port ?> \
    -T localhost:6082 \
    -f /etc/varnish/default.vcl \
    -S /etc/varnish/secret \
    -s malloc,256m"
