#!/bin/bash

echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"
echo " Destroying your development containers...              "
echo " This will eliminate your database and ALL of your source"
echo " code at aegir-home."
echo ""
echo " If you do not want this, "
echo " Hit CTRL-C now!"
echo ""
echo " Waiting 5 seconds..."
sleep 5

echo "Æ| docker-compose kill"
docker-compose kill

echo "Æ| docker-compose rm -fv"
docker-compose rm -fv

echo "Æ| sudo rm -rf aegir-home"
sudo rm -rf aegir-home

echo "==========================ÆGIR=========================="
echo "------------------------ DEVSHOP------------------------"

echo " Development environment destroyed.  "
