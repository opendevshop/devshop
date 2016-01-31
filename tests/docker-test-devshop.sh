#!/usr/bin/env bash
CONTAINER_NAME=devshop_server
docker exec $CONTAINER_NAME env TERM=xterm sudo su - aegir -c "devshop devmaster:test"