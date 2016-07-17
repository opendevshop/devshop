DevShop
============

![OpenDevShop Project Dashboard](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/devshop.png "A screenshot of the OpenDevShop Project Dashboard")

[![Join the chat at https://gitter.im/opendevshop/devshop](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/opendevshop/devshop?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Version | Status | Aegir | Hosts | DevMaster | Install & CLI 
--------|--------|-------|-------|----|-----
1.x     | Stable |3.x | D6,D7,D8 |  [![DevMaster 1.x Status](https://travis-ci.org/opendevshop/devmaster.svg?branch=1.x)](https://travis-ci.org/opendevshop/devmaster) |  [![DevShop 1.x Status](https://travis-ci.org/opendevshop/devshop.svg?branch=1.x)](https://travis-ci.org/opendevshop/devshop) 

DevShop is a "cloud hosting" system for Drupal. DevShop makes it easy to host, develop, test and update drupal sites.  It provides a front-end built in Drupal ([Devmaster](http://drupal.org/project/devmaster)) and a back-end built with drush ([DevShop Provision](http://drupal.org/project/devshop_provision)).

DevShop deploys your sites using git, and allows you to create unlimited environments for each site.  DevShop makes it very easy to deploy any branch or tag to each environment

Code is deployed on push to your git repo automatically.  Deploy any branch or tag to any environment. Data (the database and files) can be deployed between environments.  Run the built-in hooks whenever code or data is deployed, or write your own.

Resources
---------

* [General Documentation](https://devshop.readthedocs.org)  More coming soon.  Documentation is in progress.
* [Project Homepage](https://www.drupal.org/project/devshop) drupal.org/project/devshop
* [Issue Queue](https://www.drupal.org/project/issues/devshop) drupal.org/project/issues/devshop
* [Development Information](https://devshop.readthedocs.org/en/latest/help/DEVELOPMENT/)  Developer documentation will walk you through contributing to DevShop.

Tour
----

### DevShop Homepage

![OpenDevShop Homepage: Projects List](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/devshop-projects.png "A screenshot of the OpenDevShop Homeage: a clear list of all of your projects and all of your environments.")

The OpenDevShop Homepage shows you a birds evey view of all of your projects.  The name, install profile, git URL, Drupal Version, and a list of all the environments are visible at a glance.

Each environment indicator is updated in realtime. You can see the status of the latest task for every site in your system.

### DevShop Project Dashboard

![OpenDevShop Project Dashboard](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/devshop.png "A screenshot of the OpenDevShop Project Dashboard")

The project dashboard shows you all the information you need about your website.  Git URL, list of branches and tags,
links to GitHub, links to the live environment, Drush aliases, and most importantly: your project's environments.

Each block is a running copy of your website.  Name them whatever you want. Each one shows you the drupal version, the 
current git branch or tag, the URLs that are available, the last commit time, a files browser, and a backup system.

### Environments Dashboard 

![OpenDevShop Environment](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/environment-settings.png "A screenshot of an OpenDevShop Environment UI.")

Under the "Environment Settings" button is a list of possible tasks:

  - **Download Modules** allows you to add drupal modules and themes to your project, automatically committing them to git.
  - **Clone Environment** creates an exact copy of your environment with a new name.
  - **Fork Environment** runs a clone, then creates a new branch with a name of your choice!
  - **Disable & Destroy Environment**. A setting prevents environments from being destroyed in two clicks, use at your discretion. 
  - **Flush all caches**, **Rebuild Registry**, **Run Cron**,etc.  *These tasks are not really needed if you use Deploy hooks! Cron is always enabled, and caches can be cleared on every code deployment.
  - **Backup / Restore**, as you would expect.
  - **Run Tests** allows you to manually trigger test runs.
  
### Deploy

![Environment Code Deploy](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/deploy-code.png "A screenshot of the Deploy Code widget.")

 - The **Deploy Code** control allow you to easily change what branch or tag an environment is tracking. 
 - **Deploy Data** allows you to deliver new database and files to your environment. 
 - **Deploy Stack** allows you to move your environment's services (like database and files) from one server to another.

### Tasks

![Environment Task Logs](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/environment-task-logs.png "A screenshot of the Environment Task Logs.")

At the bottom of each environment block is a status indicator for the last task that was run on the environment.

You can click any task to view the detailed logs of any task.

### DevShop Logs

DevShop is designed for developers. We want to give them exactly the information they need.  No more, no less.

Out of this came the design of our task logs. We strive to make DevShop's activities as clear and transparent as possible.

![Deploy Code Logs](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/logs-deploy-pull.png "A screenshot of Deploy Code logs.")

![Deploy Hooks Logs](https://raw.githubusercontent.com/opendevshop/devshop/1.x/docs/images/logs-deploy-pull.png "A screenshot of Deploy Code logs running drush updb.")

After the code is deployed with git, any number of *Deploy Hooks* can be run.

Roadmap
-------

We are starting to track our efforts using EPICs and Huboard: [https://huboard.com/opendevshop/devshop](https://huboard.com/opendevshop/devshop)

You can browse the tag EPIC in the GitHub issues to get an idea of what efforts are underway [https://github.com/opendevshop/devshop/labels/EPIC](https://github.com/opendevshop/devshop/labels/EPIC)

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

See [the installation instructions](docs/install.md) for detailed information on installing DevShop.

Usage
-----

Using devshop is a lot like using aegir.

Visit http://devshop.local or your chosen domain in the browser to view the front-end.

SSH into your server as the `aegir` user to access the back-end.

Use drush to access any of your sites.  Use `drush sa` to see the list of available aliases.

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

Think this can be improved? You can [edit this file on GitHub](https://github.com/opendevshop/devshop/edit/1.x/README.md) and select "Create a new branch for this commit and start a pull request.".

Thanks!
