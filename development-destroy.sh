#!/bin/bash

echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
echo " Destroying your development containers...              "
echo " This will eliminate your database. If you do not want this, "
echo " Hit CTRL-C now!"
echo ""
echo " Waiting 5 seconds..."
sleep 5

echo "Æ| docker-compose kill"
docker-compose kill

echo "Æ| docker-compose rm -fv"
docker-compose rm -fv

echo "Æ| rm -rf aegir-home/.drush"
rm -rf aegir-home/.drush

echo "Æ| rm -rf aegir-home/config"
rm -rf aegir-home/config

echo "Æ| sudo rm -rf aegir-home/devmaster-1.x/sites/devshop.local.computer"
sudo rm -rf aegir-home/devmaster-1.x/sites/devshop.local.computer