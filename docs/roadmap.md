DevShop RoadMap
===============

# Past

## 0.1.x 

- First semantic versioning release.
- Completely new user interface. 
- Behat Testing.
- Install Script
- Vagrantfile.
- Travis CI
- ReadTheDocs.org setup.

## 0.2.x

- Aegir cleanups.
- UI enhancements.
- Improved Behat Testing and Logging

## 0.3.x

- DevShop CLI.
- Improved GitHub integration.
- Improved testing interfaces.
- Addition of multiple test engines.
- Major documentation efforts.

## 0.4.x

- Adding "remote:install" command and ansible playbook to setup remote servers.
- Added SSL by default.
- UI Improvements.
- Aegir SSH module.
- GitHub API integration.

# Current
## 0.5.x
 
- Major UI improvements across the system.
- Creation of the `gh-pages` branch for getdevshop.com.
- Added "Help" widget.
- "Download Modules" task: add modules to your project through the devshop web UI.
- Improvements to the "devshop upgrade" command.
- Introduced ".development_mode" for improved developer experience.
- Wrote some behat tests for testing the devshop web UI.

# In Progress
## 0.6.x 

**Hooks Improvements**

- DevShop `.hooks`: Create a .hooks file in your codebase to control what happens post-deploy, post-install, post-sync, etc.  See the [documentation](http://devshop.readthedocs.org/en/latest/deployment-hooks/) for more information
- Acquia Cloud Hooks compatibility: finalize all the hooks.

# Future

## 0.7.x

**Tasks Improvements**

- Commit Code: Allow users to commit and push code through the web UI.
- Update Features: Allow users to recreate their features modules through the web UI.
- Download Modules: Improving the module to not always commit and push.
- Rebuild: Destroy and Recreate the environment from the primary.

## 0.8.x

- Drupal 7 / Aegir 3 port: Able to host Drupal 8.

## 0.9.x

- Commit Configuration task.
- Clean up sub modules: Logs, files browser, git browser.

These are not 1.0.0 release blockers. If we decide to postpone these features to 1.1.x we can. 

# DevShop 1.0.0: Early January 2016

## 1.x

We will release 1.0.0 when we have completed the move to Aegir 3 and can successfully host Drupal 8 sites.

## 1.x.y

### DevShop Cloud

*In Progress:* Branch `devshop-cloud` 

- Provisions servers with cloud hosting providers: DigitalOcean, SoftLayer, AWS, Rackspace 

See `devshop-cloud` branch of [opendevshop/devmaster](https://github.com/opendevshop/devmaster/tree/devshop-cloud/modules/devshop/devshop_cloud) repo.

## 2.x
### Drupal 8 & Symfony

Ideally, Aegir 4 will be Symfony components with a Drupal 8 front-end.

DevShop 2 would be a flavor of that.

My hope is for Aegir 4 and DevShop 2 to be completely server and app agnostic. 