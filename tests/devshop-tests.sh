#!/bin/bash
set -e

# Use path relative to this script to find bin dir.
DEVSHOP_PATH="$( cd "$(dirname "$0")"/../bin ; pwd -P )"
echo "Found DevShop CLI in $DEVSHOP_PATH"

echo "DevShop | devshop-tests.sh | environment"
env

# Print the lines and exit if a failure happens.
echo "DevShop | devshop-tests.sh | Checking versions of devshop, drush, node, npm..."
/usr/share/devshop/bin/devshop --version
/usr/share/devshop/bin/drush --version

# Set PATH to devshop bin folder so drush, node, npm, and devshop commands are fixed.
export PATH=${DEVSHOP_PATH}:${PATH}

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

# Save GitHub Token
if [ -n "${GITHUB_TOKEN}" ]; then
  drush @hostmaster vset devshop_github_token ${GITHUB_TOKEN}
  echo ">> Drupal variable set from GITHUB_TOKEN environment variable."
else
  echo ">> GITHUB_TOKEN environment variable not found."
fi

# Run the test suite.
devshop devmaster:test
#drush @hostmaster provision-test --behat-folder-path=profiles/devmaster/tests --test-type=behat

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0
