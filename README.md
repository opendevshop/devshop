DevShop
=======

[![Build Status](https://travis-ci.org/opendevshop/devshop.svg?branch=0.x)](https://travis-ci.org/opendevshop/devshop)

DevShop is a "cloud hosting" system for Drupal. DevShop makes it easy to host, develop, test and update drupal sites.  It a provides front-end built in Drupal ([Devmaster](http://drupal.org/project/devmaster)) and a back-end built with drush ([DevShop Provision](http://drupal.org/project/devmaster)).

DevShop deploys your sites using git, and allows you to create unlimited environments for each site.  DevShop makes it very easy to deploy any branch or tag to each environment

Code is deployed on push to your git repo automatically.  Deploy any branch or tag to any environment. Data (the database and files) can be deployed between environments.  Run the built-in hooks whenever code or data is deployed, or write your own.

Resources
---------

* [General Documentation](https://devshop.readthedocs.org)  More coming soon.  Documentation is in progress.
* [Project Homepage](https://www.drupal.org/project/devshop) drupal.org/project/devshop
* [Issue Queue](https://www.drupal.org/project/issues/devshop) drupal.org/project/issues/devshop

Components
----------
DevShop currenly consists of four main components:

* [DevShop](https://github.com/opendevshop/devshop) DevShop core.  *This repository*
  * https://github.com/opendevshop/devshop
  * Install scripts.
  * Ansible playbook and roles.
  * Vagrantfile.
  * Tests (coming soon).
  * Clone this to get everything else.  
  * Use this for development.
* [Devmaster](https://www.drupal.org/project/devmaster) DevShop Front-End.  
  * https://www.drupal.org/project/devmaster
  * An install profile and makefile.
* [DevShop Hosting](https://www.drupal.org/project/devshop_hosting) DevShop Modules
  * https://www.drupal.org/project/devshop_hosting
  * To be merged into devmaster.
  * Drupal modules powering the devshop web interface.
* [DevShop Provision](https://www.drupal.org/project/devshop_provision) 
  * https://www.drupal.org/project/devshop_provision
  * To be merged into devmaster.  
  * Drush commands needed for devshop.

Support
-------

* Bug reports and feature requests should be reported in the [Drupal DevShop Issue Queue](https://www.drupal.org/project/issues/devshop).
* Join #devshop on IRC.
  
Tests
-----

We have TravisCI running to test the install script.  We have no other tests, currently.  Pull Requests welcome.

Installation
------------

See INSTALL.md for installation instructions.

Usage
-----

Using devshop is a lot like using aegir.

Visit http://devshop.local or your chosen domain in the browser to view the front-end.

SSH into your server as the `aegir` user to access the back-end.

Use drush to access any of your sites.  Use `drush sa` to see the list of available aliases.

Vagrant
-------

There is now a vagrantfile for DevShop that makes for an easy way to test it out and to contribute to the development of DevShop.

It is included in this package. To use, clone this repo and vagrant up:

See README.vagrant.md for more information.

Testing
-------

Very rudimentary testing is happening on TravisCI at http://travisci.org/opendevshop/devshop

TravisCI tests on Ubuntu 12.04, therefor 12.04 is the most supported.

The install script has been tested on:

  - ubuntu 12.04
  - centos 7.0
