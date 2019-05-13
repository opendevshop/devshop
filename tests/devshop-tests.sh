#!/bin/bash

set -e

echo ">> ENV on devshop-tests.sh:"
env

# Print the lines and exit if a failure happens.
echo ">> Checking versions of devshop, drush, node, npm..."
/usr/share/devshop/bin/devshop --version
/usr/share/devshop/bin/drush --version

# @TODO: These commands fail when using the docker-compose based test suites.
# See https://travis-ci.org/opendevshop/devshop/jobs/529898435#L2196
if [ -v $TRAVIS ]; then
    node --version
    npm --version
fi

# Run remaining tasks from install process.

echo ">> Verify hostmaster platform first."
PLATFORM_ALIAS=`drush @hm php-eval "print d()->platform->name"`
drush @hostmaster hosting-task $PLATFORM_ALIAS verify --fork=0 --strict=0 --force

echo ">> Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0 --force"
# Sometimes the task isn't queued and the command throws an error. Continue if this happens.
set +e
drush @hostmaster hosting-tasks --fork=0 --strict=0 --force
set -e

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