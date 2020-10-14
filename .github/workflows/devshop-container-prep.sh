#!/bin/bash
set -e
DEVSHOP_PATH="$( cd "$(dirname "$0")"/../../bin ; pwd -P )"
PATH="$DEVSHOP_PATH:$PATH"

devshop-logo "Preparing DevShop Control for CI Tests"

devshop-log "Adding repos to composer global config."
composer config --global repo.devshop_devmaster {"path","/usr/share/devshop/devmaster"}
cd "$DEVSHOP_PATH/src/DevShop/Components/DevShopControlTemplate"
git init
git checkout -b "${GITHUB_HEAD_REF:-1.x}"
composer require "devshop/devmaster:${GITHUB_HEAD_REF:-1.x}-dev"