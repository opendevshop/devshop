# opendevshop/devshop-install
Home of the standaone DevShop install script.

This script lives at `get.devshop.tech` and `test.devshop.tech` for easy installation.

The purpose of the install script is for a convenience for quickly
installing the latest DevShop releases on the supported linux
distros. It is not recommended to depend on this script for deployment
to production systems. For more thorough instructions for installing
on the supported distros, see the [install
instructions](https://docs.opendevshop.com/install-and-upgrade/installing-devshop).

This repository is solely maintained by ThinkDrop, Inc.

## Usage:

From `https://get.devshop.tech`:
```shell
curl -fsSL https://get.devshop.tech -o install.sh
sh install.sh --hostname=devshop.myhostname.com
```

## Credit

This repository is modelled after the [get.docker.com](https://get.docker.com) script [repository located on GitHub](https://github.com/docker/docker-install).

A new install.sh script will be created using Docker's install.sh as a template.

Until that script is ready, this repo will host the legacy DevShop install.sh script.
