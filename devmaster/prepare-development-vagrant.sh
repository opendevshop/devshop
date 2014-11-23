#!/bin/bash

# We must add www-data to the aegir group in vagrant because the synced folder
# /var/aegir/devshop-6.x-1.x (devshop front-end) is owned by aegir.
sudo adduser www-data aegir
sudo service apache2 restart
