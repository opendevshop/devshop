#!/bin/bash

set -e

# Run remaining tasks from install process.
echo ">> Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0 --force"
drush @hostmaster hosting-tasks --fork=0 --strict=0 --force

echo ">> Running remaining tasks: Complete!"

# Pause the task queue.
drush @hostmaster dis hosting_queued -y
drush @hostmaster vset hosting_queued_paused 1

# Enable watchdog
drush @hostmaster en dblog -y

# Run the test suite.
/usr/share/devshop/bin/devshop devmaster:test
#drush @hostmaster provision-test --behat-folder-path=profiles/devmaster/tests --test-type=behat

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0