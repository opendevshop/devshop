#!/bin/bash

set -ex

echo "Running  /usr/share/devshop/bin/devshop devmaster:test"

# Pause the task queue.
drush @hostmaster vset hosting_queued_paused 1

# Run the test suite.
/usr/share/devshop/bin/devshop devmaster:test

# Unpause the task queue.
drush @hostmaster vset hosting_queued_paused 0