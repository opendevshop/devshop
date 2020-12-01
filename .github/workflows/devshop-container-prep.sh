#!/bin/bash
set -e
DEVSHOP_PATH="$( cd "$(dirname "$0")"/../.. ; pwd -P )"
PATH="$DEVSHOP_PATH/bin:$PATH"
GIT_REF=${GITHUB_HEAD_REF:-"1.x"}

devshop-logo "Preparing DevShop Control for CI Tests"
devshop-log "Creating local branch from the checked out commit with the expected name..."

# Create a local branch in the "path" repo so that composer doesn't get all confused. When the path is a SHA, composer reads the version as `devshop/devmaster[dev-8207dc45845c7cc0e4e8271d9c097fcb108a5773]`
# Assumption: Having a local branch will make composer detect the devshop/devmaster repo as being at version `devshop/devmaster[1.x-dev]`.
# I discovered this is happening by noticing that local composer install fails had a different error message:
# In GitHub Actions: https://github.com/opendevshop/devshop/pull/629/checks?check_run_id=1392567826#step:4:227
#   - Root composer.json requires devshop/devmaster ^1.7@alpha||1.x-dev, it is satisfiable by devshop/devmaster[1.7.0-alpha1, 1.7.0-alpha2, 1.7.0-alpha3, 1.x-dev] from composer repo (https://repo.packagist.org) but devshop/devmaster[dev-8207dc45845c7cc0e4e8271d9c097fcb108a5773] from path repo (/usr/share/devshop/devmaster) has higher repository priority.
# Locally, it says `devshop/devmaster[1.x-dev]` instead, because my local path repo is on a branch.

cd "$DEVSHOP_PATH"
git checkout -b 1.x || echo "1.x branch already exists."

devshop-log "Adding repos to composer global config."
set -x
composer config --global repo.devshop_devmaster {"path","/usr/share/devshop/devmaster"}
cd "$DEVSHOP_PATH/src/DevShop/Component/DevShopControlTemplate"
git log -2
git init
git checkout -b $GIT_REF
git remote add origin https://github.com/devshop-packages/devshop-control-template
git add .gitignore
git add -A
git config --global user.email "github@opendevshop.com"
git config --global user.name "GitHub Actions"
git commit -m 'Temporary commit'

# Composer require @dev and search for the Symlinking statement to ensure we are always installing from local code.
echo "Reinstalling local devmaster using composer..."
composer uninstall

# Workaround for PATH bug: If composer install fails, run again but confirm path repo and show a message.
COMPOSER_INSTALL_CMD="composer update --ansi --no-progress"
if $COMPOSER_INSTALL_CMD; then
    echo "Composer Install successfully the first time."
else
    echo "Composer Install failed the first time. This is a known bug when using the 'path' repository for devshop/devmaster. Running composer install again..."
    if $COMPOSER_INSTALL_CMD; then
      echo "Composer Install succeeded the second time!"
    else
      echo "Composer Install failed the second time too!"
      exit 1
    fi
fi
