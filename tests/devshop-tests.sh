#!/bin/bash

set -ev

# Run remaining tasks from install process.
echo ">> Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0"
drush @hostmaster hosting-tasks --fork=0 --strict=0

echo ">> Running remaining tasks: Complete!"

if [[ $* == *--upgrade* ]]; then
      echo ">> Triggering Upgrade: Running drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y"

      # Force all tasks to appear as completed.'
      drush @hostmaster sql-query "UPDATE hosting_task SET task_status = 1;"

      drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y

      echo ">> Upgrade Complete."
fi

# Pause the task queue.
drush @hostmaster vset hosting_queued_paused 1

# Run the test suite.
/usr/share/devshop/bin/devshop devmaster:test
#drush @hostmaster provision-test --behat-folder-path=profiles/devmaster/tests --test-type=behat

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0