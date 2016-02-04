#!/usr/bin/env bash
VERSION='0.x'
DISTRIBUTION='ubuntu'
VERSION='14.04'
INIT='/sbin/init'
CONTAINER_NAME='devshop_server'
RUN_OPTS="--name=$CONTAINER_NAME"
SCRIPT_OPTS='--server-webserver=nginx'
CONTAINER_HOSTNAME=devshop.docker
SUPERVISOR_STOP='service supervisor stop'
HOST_PORT=8000
TRAVIS=true

echo "Running 'vagrant-prepare-host.sh' to get source code..."
bash vagrant-prepare-host.sh $PWD $VERSION

# Changing UID:GID of source code to Aegir's UID so it can write to these folders.
# We can change it back to the user later so they can edit the files.
sudo chown 12345:12345 source -R
cd tests

composer install

# Pulled from our .travis.yml
docker pull $DISTRIBUTION:$VERSION
docker build --rm=true --file=Dockerfile.$DISTRIBUTION-$VERSION --tag=$DISTRIBUTION-$VERSION:devmaster .
docker run --detach -p $HOST_PORT:80 $RUN_OPTS \
    --volume=$PWD/..:/usr/share/devshop:rw \
    --volume=$PWD/../source/devmaster-0.x:/var/aegir/devmaster-0.x \
    --volume=$PWD/../source/drush/commands:/var/aegir/.drush/commands \
    -h $CONTAINER_HOSTNAME $DISTRIBUTION-$VERSION:devmaster $INIT
docker exec --tty $CONTAINER_NAME env TERM=xterm sudo su -c "/usr/share/devshop/install.sh $SCRIPT_OPTS --hostname=$CONTAINER_HOSTNAME"
docker exec $CONTAINER_NAME $SUPERVISOR_STOP
docker exec $CONTAINER_NAME env sudo su - aegir -c "drush @hostmaster dis hosting_queued -y -v"

bash docker-test-devshop.sh


echo "NEXT: Please add '127.0.0.1 devshop.docker' to your /etc/hosts file."
echo " You must access the site at http://devshop.docker:$HOST_PORT "
echo " To get into the server, run:
echo "    $ docker exec -ti devshop_server su - aegir "
echo " Then to get into the front end, run:"
echo "    $ devshop login "
echo " You will get a URL like http://devshop.docker/user/login/13abc.  you will have to add the port $HOST_PORT."
echo " Good luck! We hope to improve on this soon.  Please post issues at https://github.com/opendevshop/devshop/issues"