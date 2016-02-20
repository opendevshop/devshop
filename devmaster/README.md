DevShop DevMaster
=================

This is the DevShop web-based front-end, called Devmaster.

The stable branch, 0.x is a Drupal 6 Distribution.
The next branch, 1.x is a Drupal distribution.  It is mostly working, and will see a release soon.

This project should not be used on it's own.  The main devshop project 
installer will use this install profile as a part of the setup process.

See http://github.com/opendevshop/devshop for more information.

Version | Status | Aegir | Hosts | DevMaster | Install & CLI 
--------|--------|-------|-------|----|-----
0.x     | Stable |   2.x   |  D6, D7     | [![DevMaster 0.x Status](https://travis-ci.org/opendevshop/devmaster.svg?branch=0.x)](https://travis-ci.org/opendevshop/devmaster) | [![DevShop 0.x Status](https://travis-ci.org/opendevshop/devshop.svg?branch=0.x)](https://travis-ci.org/opendevshop/devshop) 
1.x     | In Development |3.x | D6,D7,D8 |  [![DevMaster 1.x Status](https://travis-ci.org/opendevshop/devmaster.svg?branch=1.x)](https://travis-ci.org/opendevshop/devmaster) |  [![DevShop 1.x Status](https://travis-ci.org/opendevshop/devshop.svg?branch=1.x)](https://travis-ci.org/opendevshop/devshop) 

Contribution
============

To contribute to this project, please fork this repo, do your work in a separate
branch and submit a Pull Request.

Travis-CI.org is configured to run tests on all pull requests for devmaster.

Contents
========

This project contains:

1. devmaster.make

  The makefile used to build devmaster.

2. devmaster.profile

  The Drupal installation profile for Devmaster.
  
3. DevShop modules:

  All of the modules needed for devshop are contained in this repo, with the 
  exception of contrib modules that are also useful for Aegir.

4. The DevShop theme "Boots":

  The theme for devshop is contained in this repo.

Issues & Development
====================

This repo may be forked if you wish to contribute to development.  

See DEVELOPMENT.md in the main devshop project for more information.
 
Issues for devshop or devmaster should be submitted to the github issue queue:

https://github.com/opendevshop/devshop/issues
