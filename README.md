DevShop
=======

Welcome to the DevShop Source code.

[![Build Status](https://travis-ci.org/opendevshop/devshop.svg?branch=0.x)](https://travis-ci.org/opendevshop/devshop)

Contents
--------

This project contains four important components to DevShop

1. install.sh
  A bash script to go from zero to DevShop.  All you need is a new server and this script.
  This script is designed to work all by itself, and is not dependent on the other files in this repo to run.

2. build-devshop.make
  The makefile used to build the DevShop front-end.

3. Ansible Playbooks
  We use ansible to provision our servers.  See playbook.yml and the "roles" folder.

4. Vagrantfile
  Allows devshop to be launched with Vagrant. Used for testing and development. See README.vagrant.md for more info.

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
