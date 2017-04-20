#!/bin/bash

if [[ $* == *--upgrade* ]]; then

      set -ex
      echo "HELLO UPGRADE >>>>>>>>>>>>>>>>>"
      exit 100
      drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y
fi

set -ex

echo "run-tests.sh | Running  /usr/share/devshop/bin/devshop devmaster:test"

# Pause the task queue.
drush @hostmaster vset hosting_queued_paused 1

# Run the test suite.
/usr/share/devshop/bin/devshop devmaster:test

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0