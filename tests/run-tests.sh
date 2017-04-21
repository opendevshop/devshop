#!/bin/bash

set -e

# Run remaining tasks from install process.
echo ">> Running remaining tasks: drush @hostmaster hosting-tasks -v --debug --fork=0"
drush @hostmaster hosting-tasks -v --debug --fork=0

if [[ $* == *--upgrade* ]]; then
      echo ">> Triggering Upgrade: Running drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y"
      drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y -v --debug

      echo ">> Upgrade Complete."
fi

set -ex

# Pause the task queue.
drush @hostmaster vset hosting_queued_paused 1

# Run the test suite.
/usr/share/devshop/bin/devshop devmaster:test

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0