#!/bin/bash

set -e

# Run remaining tasks from install process.
echo "# Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0"
drush @hostmaster hosting-tasks --fork=0 --strict=0

echo "# Running remaining tasks: Complete!"

if [[ $* == *--upgrade* ]]; then
      echo "# Triggering Upgrade: Running drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y"

      # Force all tasks to appear as completed.'
      drush @hostmaster sql-query "UPDATE hosting_task SET task_status = 1;"

      drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y

      echo "# Upgrade Complete."
fi

#set -ex

# Pause the task queue.
echo "# Running 'drush @hostmaster vset hosting_queued_paused 1'..."
drush @hostmaster vset hosting_queued_paused 1

# Run the test suite.
echo "# Running '/usr/share/devshop/bin/devshop devmaster:test'..."
/usr/share/devshop/bin/devshop devmaster:test

# Unpause the task queue.
echo "# Running 'drush @hostmaster vset hosting_queued_paused 0'..."
drush @hostmaster vset hosting_queued_paused 0