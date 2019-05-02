#!/bin/bash

set -e

# confirm devshop, drush, npm and node are executable.
set +x
devshop --version
drush --version
node --version
npm --version
set -x

# Run remaining tasks from install process.

echo ">> Verify hostmaster platform first."
PLATFORM_ALIAS=`drush @hm php-eval "print d()->platform->name"`
drush @hostmaster hosting-task $PLATFORM_ALIAS verify --fork=0 --strict=0 --force

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