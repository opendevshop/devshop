#!/bin/bash

set -e

# Run remaining tasks from install process.
echo ">> Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0"
drush @hostmaster hosting-tasks --fork=0 --strict=0

echo ">> Running remaining tasks: Complete!"

echo ">> Triggering Upgrade: Running drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y"

# Force all tasks to appear as completed.'
echo ">> Resetting all tasks..."
drush @hostmaster sql-query "SELECT * FROM hosting_task;"
drush @hostmaster sql-query "UPDATE hosting_task SET task_status = 1;"

drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y -v

echo ">> Upgrade Complete."

bash /usr/share/devshop/tests/devshop-tests.sh