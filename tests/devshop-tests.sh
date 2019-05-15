#!/bin/bash

set -e

echo "DevShop | devshop-tests.sh | environment"
env

# Print the lines and exit if a failure happens.
echo "DevShop | devshop-tests.sh | Checking versions of devshop, drush, node, npm..."
/usr/share/devshop/bin/devshop --version
/usr/share/devshop/bin/drush --version

# @TODO: These commands fail when using the docker-compose based test suites.
# See https://travis-ci.org/opendevshop/devshop/jobs/529898435#L2196
if [ -v $TRAVIS ]; then
    node --version
    npm --version
fi

# Run remaining tasks from install process.

# Pause the task queue.
echo "DevShop | devshop-tests.sh | Disabling hosting queue..."
drush @hostmaster dis hosting_queued -y
drush @hostmaster vset hosting_queued_paused 1

echo "DevShop | devshop-tests.sh | Verify hostmaster platform first..."
PLATFORM_ALIAS=`drush @hm php-eval "print d()->platform->name"`
drush @hostmaster hosting-task $PLATFORM_ALIAS verify --fork=0 --strict=0 --force

echo "DevShop | devshop-tests.sh |  Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0 --force || true"
drush @hostmaster hosting-tasks --fork=0 --strict=0 --force || true

echo "DevShop | devshop-tests.sh | Running remaining tasks: Complete!"

# Enable watchdog
drush @hostmaster en dblog -y

# Run the test suite.
/usr/share/devshop/bin/devshop devmaster:test
#drush @hostmaster provision-test --behat-folder-path=profiles/devmaster/tests --test-type=behat

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0