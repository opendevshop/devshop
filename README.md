DevShop DevMaster
=================

This is the DevShop web-based front-end, called Devmaster.

The stable branch, 0.x is a Drupal 6 Distribution.
The next branch, 1.x is a Drupal distribution.  It is mostly working, and will see a release soon.

This project should not be used on it's own.  The main devshop project 
installer will use this install profile as a part of the setup process.

See http://github.com/opendevshop/devshop for more information.

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
