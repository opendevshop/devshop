#!/bin/bash

set -e

# Run remaining tasks from install process.
echo ">> Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0"
drush @hostmaster hosting-tasks --fork=0 --strict=0 --force -v

echo ">> Running remaining tasks: Complete!"

# Force all tasks to appear as completed.'
echo ">> Checking Processing or queued tasks: "
drush @hostmaster sql-query "SELECT COUNT(nid) FROM hosting_task t WHERE task_status = -1 OR task_status = 0 GROUP BY nid ORDER BY vid DESC;"
sleep 3

echo ">> Checking Processing or queued tasks: "
drush @hostmaster sql-query "SELECT COUNT(nid) FROM hosting_task t WHERE task_status = -1 OR task_status = 0 GROUP BY nid ORDER BY vid DESC;"
sleep 3

echo ">> Checking Processing or queued tasks: "
drush @hostmaster sql-query "SELECT COUNT(nid) FROM hosting_task t WHERE task_status = -1 OR task_status = 0 GROUP BY nid ORDER BY vid DESC;"
sleep 3

echo ">> All TASKS:"
drush @hostmaster sql-query "SELECT * FROM hosting_task;"


#echo ">> Spawing hosting queued in a new process so tasks run during upgrade/migrate..."
#drush @hostmaster en hosting_queued -y
#drush @hostmaster hosting-queued &

echo ">> Triggering Upgrade: Running drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y"
drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y -v

echo ">> Upgrade Complete."

drush @hostmaster dis hosting_queued -y

bash /usr/share/devshop/tests/devshop-tests.sh