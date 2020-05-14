#!/usr/bin/env bash
ROOT_PATH="$( cd "$(dirname "$0")"/.. ; pwd -P )"

IMAGE=base
OS=${OS:-ubuntu1804}

echo "Destroying existing container in 3 seconds..."
sleep 3
docker kill devshop-${IMAGE}-${OS}

set -e

# BASE
# devshop/base is FROM $OS
docker build . --file=base/Dockerfile.${OS} --tag devshop/base:${OS}

# ANSIBLE
# devshop/ansible is FROM devshop/base
docker build . --file=ansible/Dockerfile.${OS} --tag devshop/ansible:${OS}

# DEVSHOP
# devshop/server is FROM devshop/ansible
# @TODO: Build new devshop/server dockerfiles.
# docker build . --file=server/Dockerfile.centos7 --tag devshop/server:centos7
# docker build . --file=server/Dockerfile.ubuntu1804 --tag devshop/server:ubuntu1804

DEVSHOP_SCRIPT_PATH=/root/install.sh
DEVSHOP_SOURCE_PATH=/usr/share/devshop
HOSTNAME=devshop.local.computer

# Same as what's in github actions
docker run \
    --name devshop-${IMAGE}-${OS} \
    --rm \
    --detach \
    --privileged \
    --volume /sys/fs/cgroup:/sys/fs/cgroup:ro \
    --volume "${ROOT_PATH}/:${DEVSHOP_SOURCE_PATH}" \
    --volume /var/lib/mysql \
    --publish "80:80" \
    --hostname ${HOSTNAME} \
    devshop/${IMAGE}:${OS}

docker exec -ti devshop-${IMAGE}-${OS} \
    bash ${DEVSHOP_SOURCE_PATH}/install/install.sh