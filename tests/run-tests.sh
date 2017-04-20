#!/bin/bash

set -ex

if [[ $* == *--upgrade* ]]; then
      echo "Triggering Upgrade: Running drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y"
      drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y
fi

# Pause the task queue.
drush @hostmaster vset hosting_queued_paused 1

# Run the test suite.
/usr/share/devshop/bin/devshop devmaster:test

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0