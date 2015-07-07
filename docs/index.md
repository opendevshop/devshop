Open DevShop
============

<img src="https://www.drupal.org/files/devshop.png" width="100%">

[![Join the chat at https://gitter.im/opendevshop/devshop](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/opendevshop/devshop?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Build Status](https://travis-ci.org/opendevshop/devshop.svg?branch=0.x)](https://travis-ci.org/opendevshop/devshop)

DevShop is a "cloud hosting" system for Drupal. DevShop makes it easy to host, develop, test and update drupal sites.  It a provides front-end built in Drupal ([Devmaster](http://drupal.org/project/devmaster)) and a back-end built with drush ([DevShop Provision](http://drupal.org/project/devshop_provision)).

DevShop deploys your sites using git, and allows you to create unlimited environments for each site.  DevShop makes it very easy to deploy any branch or tag to each environment

Code is deployed on push to your git repo automatically.  Deploy any branch or tag to any environment. Data (the database and files) can be deployed between environments.  Run the built-in hooks whenever code or data is deployed, or write your own.

Resources
---------

* [General Documentation](https://devshop.readthedocs.org)  More coming soon.  Documentation is in progress.
* [Project Homepage](https://www.drupal.org/project/devshop) drupal.org/project/devshop
* [Issue Queue](https://www.drupal.org/project/issues/devshop) drupal.org/project/issues/devshop
* [Development Information](https://devshop.readthedocs.org/en/latest/help/DEVELOPMENT/)  Developer documentation will walk you through contributing to DevShop.

Components
----------
DevShop currenly consists of four main components:

**DevShop**

DevShop core.  *This repository*

  * https://github.com/opendevshop/devshop
  * Install scripts.
  * Ansible playbook and roles.
  * Vagrantfile.
  * Tests (coming soon).
  * Clone this to get everything else.  
  * Use this for development.

**Devmaster**

DevShop Front-End.  

  * https://github.com/opendevshop/devmaster
  * An install profile and makefile.
  * DevShop Drupal modules

**DevShop Provision**

DevShop Drush commands.

  * https://github.com/opendevshop/devshop_provision
  * To be merged into devmaster.  
  * Drush commands needed for devshop.

Support
-------

* Bug reports and feature requests should be reported in the [Drupal DevShop Issue Queue](https://www.drupal.org/project/issues/devshop).
* Join #devshop on IRC.


Installation
------------

See INSTALL.md for installation instructions.

Usage
-----

Using devshop is a lot like using aegir.

Visit http://devshop.local or your chosen domain in the browser to view the front-end.

SSH into your server as the `aegir` user to access the back-end.

Use drush to access any of your sites.  Use `drush sa` to see the list of available aliases.

Versions
--------

We have two active branches as we try to reach for Drupal 8 hosting in time for release.  1.x has just started, and isn't functional yet.  Please use the 0.x branch of DevShop.

DevShop version | Branch | DevMaster Versions | Hosted Drupal Versions | Aegir |Status
----------------|--------|--------------------|------------------------|-------|-------
0.3.1 *CURRENT* | 0.x |  6.x-1.x           | 6, 7                   | 2.x | Stable
1.0.0 *coming later*| 1.x |  7.x-1.x           | 6, 7, 8                | 3.x | non-functional

Vagrant
-------

There is now a vagrantfile for DevShop that makes for an easy way to test it out and to contribute to the development of DevShop.

It is included in this package. To use, clone this repo and vagrant up.

### Vagrant Development Mode

By default, vagrant development mode is on.  This is set in `vars.yml`:

```
# Set development to FALSE if you wish to test a "clean" devshop install.
vagrant_development: true
```

If vagrant development is set to TRUE, then the script `vagrant-prepare-host.sh` is run on the first call to `vagrant up`.

This script requires drush and git to be installed on the host, so that we can build devmaster and clone the repos locally.

The source files are cloned into the `/source` folder in this repo, which is mounted inside the vagrant box.  Once up and running, you can edit any files in the `/source` folder and it will be immediately visible in the VM.

Testing
-------

Very rudimentary testing is happening on TravisCI at http://travisci.org/opendevshop/devshop

TravisCI tests on Ubuntu 12.04, therefor 12.04 is the most supported.

The install script has been tested on:

  - ubuntu 12.04
  - centos 7.0

License
-------

DevShop is licensed under [GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt).

This means any forks of this code must be released as open source and also be licensed under the GPL.

Help Improve Documentation
--------------------------

Think this can be improved? You can [edit this file on GitHub](https://github.com/opendevshop/devshop/edit/0.x/README.md) and select "Create a new branch for this commit and start a pull request.".

Thanks!
