#!/bin/bash
set -e
DEVSHOP_PATH="$( cd "$(dirname "$0")"/../.. ; pwd -P )"
PATH="$DEVSHOP_PATH/bin:$PATH"
GIT_REF=${GITHUB_HEAD_REF:-"1.x"}

devshop-logo "Preparing DevShop Control for CI Tests"

devshop-log "Adding repos to composer global config."
set -x
composer config --global repo.devshop_devmaster {"path","/usr/share/devshop/devmaster"}
cd "$DEVSHOP_PATH/src/DevShop/Component/DevShopControlTemplate"
git init
git checkout -b $GIT_REF
composer require "devshop/devmaster:$GIT_REF-dev" --no-progress --no-suggest
