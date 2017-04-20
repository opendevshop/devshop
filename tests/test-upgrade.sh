#!/bin/bash

set -ex
drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y

drush @hostmaster vset hosting_queued_paused 1

# Run the test suite.
#cd ${AEGIR_HOSTMASTER_ROOT_TARGET}/profiles/devmaster/tests

cd /usr/share/devshop/tests

composer install
bin/behat --profile=devmaster
