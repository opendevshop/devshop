#!/bin/bash

set -e

# Run remaining tasks from install process.
echo ">> Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0"
drush @hostmaster hosting-tasks --fork=0 --strict=0 --force -v

echo ">> Running remaining tasks: Complete!"

# Force all tasks to appear as completed.'
echo ">> Waiting for tasks to complete... "

tasks_ready() {
  echo "Running drush hosting-tasks..."
  drush @hostmaster hosting-tasks --fork=0 --strict=0 --force -v

  COUNT=`drush @hostmaster eval "print hosting_task_count()"`
  COUNT_RUNNING=`drush @hostmaster eval "print hosting_task_count_running()"`

  echo "$COUNT tasks still queued, $COUNT_RUNNING running."

  if [ $COUNT == "0" ] && [ $COUNT_RUNNING == "0" ]; then
    echo "No Tasks Left!"
    exit 0
  else
    echo "Tasks are left. Not yet Ready."
    exit 1
  fi

}

while !(tasks_ready)
do
   sleep 3
   echo "ÆGIR | Checking tasks..."
done



#echo ">> Spawing hosting queued in a new process so tasks run during upgrade/migrate..."
#drush @hostmaster en hosting_queued -y
#drush @hostmaster hosting-queued &

echo ">> Triggering Upgrade: Running drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y"
drush @hostmaster hostmaster-migrate $HOSTNAME $AEGIR_HOSTMASTER_ROOT_TARGET -y -v

echo ">> Upgrade Complete."

drush @hostmaster dis hosting_queued -y

bash /usr/share/devshop/tests/devshop-tests.sh