#!/usr/bin/env bash
DEVSHOP_VERSION='0.x'
DISTRIBUTION='ubuntu'
DISTRIBUTION_VERSION='14.04'
INIT='/sbin/init'
CONTAINER_NAME='devshop_server'
RUN_OPTS="--name=$CONTAINER_NAME"
SCRIPT_OPTS="--server-webserver=nginx --aegir_user_uid=$UID"
CONTAINER_HOSTNAME=devshop.docker
SUPERVISOR_STOP='service supervisor stop'
HOST_PORT=8000
TRAVIS=true

# Do NOT let this script run as root.
if [[ ! $EUID -ne 0 ]]; then
   echo "This script must NOT be run as root. Run it as the same user you edit your files with."
   echo "This user must also be able to run the 'docker' command."
   echo "If it cannot, try adding it to the docker group: "
   echo "  $ sudo usermod -aG docker yourusername  "
   exit 1
fi

echo "Running 'vagrant-prepare-host.sh' to get source code..."
bash vagrant-prepare-host.sh $PWD $DEVSHOP_VERSION

# Changing UID:GID of source code to Aegir's UID so it can write to these folders.
# We can change it back to the user later so they can edit the files.
cd tests

composer install

# Pulled from our .travis.yml
docker pull $DISTRIBUTION:$DISTRIBUTION_VERSION
docker build --rm=true --file=Dockerfile.$DISTRIBUTION-$DISTRIBUTION_VERSION --tag=$DISTRIBUTION-$DISTRIBUTION_VERSION:devmaster .
docker run --detach -p $HOST_PORT:80 $RUN_OPTS \
    --volume=$PWD/..:/usr/share/devshop:rw \
    --volume=$PWD/../source/devmaster-$DEVSHOP_VERSION:/var/aegir/devmaster-$DEVSHOP_VERSION \
    --volume=$PWD/../source/drush/commands:/var/aegir/.drush/commands \
    -h $CONTAINER_HOSTNAME $DISTRIBUTION-$DISTRIBUTION_VERSION:devmaster $INIT
docker exec --tty $CONTAINER_NAME env TERM=xterm sudo su -c "/usr/share/devshop/install.sh $SCRIPT_OPTS --hostname=$CONTAINER_HOSTNAME"
docker exec $CONTAINER_NAME $SUPERVISOR_STOP
docker exec $CONTAINER_NAME env sudo su - aegir -c "drush @hostmaster dis hosting_queued -y -v"

bash docker-test-devshop.sh

echo ""
echo "NEXT: Please add '127.0.0.1 devshop.docker' to your /etc/hosts file."
echo " You must access the site at http://devshop.docker:$HOST_PORT "
echo " To get into the server, run: "
echo "    $ docker exec -ti devshop_server su - aegir "
echo " Then to get into the front end, run:"
echo "    $ devshop login "
echo " You will get a URL like http://devshop.docker/user/login/13abc.  you will have to add the port $HOST_PORT."
echo " Good luck! We hope to improve on this soon.  Please post issues at https://github.com/opendevshop/devshop/issues "
echo " "
echo " NOTE: To run tests, we have disabled supervisor.  To run the task queue runner, run: "
echo " docker exec -ti devshop_server su - aegir -c 'drush @hostmaster en hosting_queued -y && drush @hostmaster hosting-queued' "