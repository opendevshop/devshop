#!/bin/bash
set -e
DEVSHOP_PATH="$( cd "$(dirname "$0")"/../bin ; pwd -P )"
PATH="$DEVSHOP_PATH:$PATH"

devshop-logo "Running DevShop Tests"
log echo "Path: $PATH"
devshop status

# Print the lines and exit if a failure happens.
log "Checking versions of devshop, drush, node, npm..."
log "DevShop Version: $(devshop --version)"
log "Drush Version: $(drush --version)"

#echo "Node Version:     " && node --version
#echo "NPM Version:      " && npm --version

# Run remaining tasks from install process.
# Pause the task queue.
log "Disabling hosting queue..."
drush @hostmaster dis hosting_queued -y
drush @hostmaster vset hosting_queued_paused 1

#echo "DevShop | devshop-tests.sh | Verify hostmaster platform first..."
#PLATFORM_ALIAS=`drush @hm php-eval "print d()->platform->name"`
#drush @hostmaster hosting-task $PLATFORM_ALIAS verify --fork=0 --strict=0 --force

log "Running remaining tasks: drush @hostmaster hosting-tasks --fork=0 --strict=0 --force || true"
drush @hostmaster hosting-tasks --fork=0 --strict=0 --force || true

log "Running remaining tasks: Complete!"

# Enable watchdog
drush @hostmaster en dblog -y

# Save GitHub Token
if [ -n "${GITHUB_TOKEN}" ]; then
  drush @hostmaster vset devshop_github_token ${GITHUB_TOKEN}
  log "Drupal variable set from GITHUB_TOKEN environment variable."
else
  log "WARNING: GITHUB_TOKEN environment variable not found."
fi

# Run the test suite.
devshop devmaster:test
#drush @hostmaster provision-test --behat-folder-path=profiles/devmaster/tests --test-type=behat

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0
