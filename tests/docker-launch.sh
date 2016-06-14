#!/usr/bin/env bash

DISTRIBUTION='ubuntu'
VERSION='14.04'
INIT='/sbin/init'
CONTAINER_NAME='devshop_server'
RUN_OPTS="--name=$CONTAINER_NAME"
SCRIPT_OPTS='--server-webserver=nginx'
CONTAINER_HOSTNAME=devshop.docker

# Don't stop supervisor until we want to run tests.
# SUPERVISOR_STOP='service supervisor stop'
HOST_PORT=8000
TRAVIS=true

# Create an inventory file so we can set some variables
echo "$CONTAINER_HOSTNAME aegir_user_uid=$UID aegir_user_gid=$UID" > ../inventory

# Pulled from our .travis.yml
docker pull $DISTRIBUTION:$VERSION
docker build --rm=true --file=Dockerfile.$DISTRIBUTION-$VERSION --tag=$DISTRIBUTION-$VERSION:devmaster .
docker run --detach -p $HOST_PORT:80 $RUN_OPTS --volume=$PWD/..:/usr/share/devshop:rw -h $CONTAINER_HOSTNAME $DISTRIBUTION-$VERSION:devmaster $INIT

# Run install.sh
docker exec --tty $CONTAINER_NAME env TERM=xterm sudo su -c "/usr/share/devshop/install.sh $SCRIPT_OPTS --hostname=$CONTAINER_HOSTNAME"

# Don't stop queue until the user runs tests.
# docker exec $CONTAINER_NAME $SUPERVISOR_STOP
# docker exec $CONTAINER_NAME env sudo su - aegir -c "drush @hostmaster dis hosting_queued -y -v"
# bash docker-test-devshop.sh
