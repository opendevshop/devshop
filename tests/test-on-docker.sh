#!/bin/bash
set -e
DEVSHOP_TEST_SCRIPTS_PATH=$(dirname "$0")
DEVSHOP_PATH="$( cd "$(dirname "$0")"/../bin ; pwd -P )"

echo "Adding $DEVSHOP_PATH to PATH variable..."
PATH="$DEVSHOP_PATH:$PATH"

DEVSHOP_USER="aegir"

ansible-playbook /etc/ansible/play.yml --tags runtime

exec su --preserve-environment ${DEVSHOP_USER} --command ${DEVSHOP_TEST_SCRIPTS_PATH}/devshop-tests.sh