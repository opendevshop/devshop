#!/usr/bin/env bash
CONTAINER_NAME=devshop_server

composer install

docker exec $CONTAINER_NAME service supervisor stop
docker exec $CONTAINER_NAME env sudo su - aegir -c "drush @hostmaster dis hosting_queued -y -v"

docker exec $CONTAINER_NAME env TERM=xterm sudo su - aegir -c "devshop devmaster:test"