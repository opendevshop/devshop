# Change Log

## 0.5.1 (September 22, 2015)

- Fixing a bug caused by our move away from the path alias "hosting/c/NAME": hosting_context_register() saves the context AND sets the path alias. We must do both.

## 0.5.0 (September 22, 2015)

- Created new dedicated "environment" template file and improved theming.
- Added task status display on environments for all tasks (not just tests).
- Saving "last task" status for every environment.
- Implemented new theme on "site/environment" nodes, adding task logs, devshop actions.
- Improved "Login" link: Only one click is needed.
- Improving overall theme with left-floated "tabs".
- Cleanup "environment settings" page, going back to using site node edit page.
- Major redesign of tasks pages:
   - More visible, easier to access "Retry" button.
   - Raise error or warning messages to top of page.
- Major improvements to the main projects homepage: Now showing all environments and their status.
- Adding "Aegir Download" module, allowing adding modules to project repos in one click.
- Tiny change to the logo.
- Fixes a problem with cloning sites for pull requests: if files moved, an error would throw, clone would roll back.  Now, a registry rebuild runs no matter what when cloning an environment during a pull request.
- Added drush command "platform-delete" to allow command line removal of platforms from hostmaster.  Used mainly for the devshop upgrade process.
- When a pull request is deleted and there is no site, trigger deleting of the platform.
- When receiving a pull request notification, if the branch is behind master, GitHub API will return a failure.  When this happens, we notify the pull request.
- All environment task status displays now change dynamically!
- Adding logo as default theme logo and favicon.
- Making "development mode" easier to set.
- Improving upgrade process, allowing for multiple 0.x updates, and automatically queuing the deletion of old devmaster platforms.
- Adding PHP configs post_max_size and upload_max_filesize as ansible variables.

## 0.4.1 (September 12, 2015)

- Defaulting the Vagrant host to http://devshop.site
- Turning off "development mode" by default. If you are a devshop developer, check vars.yml to make sure you can edit code.
- Fixing a bug in the install script.
- Fixing another bug in the update command.
- Added documentation page for troubleshooting a stalled task queue. (Thanks @jdixon567)
- Added documentation page for upgrading devshop.

## 0.4.0 (September 9, 2015)

101 commits to DevShop: https://github.com/opendevshop/devshop/compare/0.3.1...release-0.4.0
179 commits to Devmaster: https://github.com/opendevshop/devmaster/compare/0.3.1...release-0.4.0

- Adding "remote" machines to vagrantfile for testing remotes and clusters.
- Created "devshop remote:install" command and ansible playbook, making setup of additional remote servers simple.
- Created "devshop upgrade" command to make upgrading a devshop server simple.
- Fixing issues with installation on RedHat/CentOS systems.
- Install SSL by default.
- Adding documentation on installing remote machines setup, SSL, scaling.
- Environment UI overhaul. Last task log status display, separated actions from task logs drop downs, better login link, etc.
- Dynamic one click login for environments.
- Improved Projects UI on sites, platforms, and task node pages.
- Fixed a bug preventing data migration between remote servers.
- Fixed a bug when adding domain aliases.
- Added Aegir SSH module allowing users to upload their SSH keys to the server to get access to drush.
- Fixed the awkward primary links order and adding user menu to secondary links.
- On project delete confirmation form, show every environment that will be deleted.
- Redesign github pull request environments UI.
- Improved environment (site) node page. (Still needs work. Next release.)
- Fixes improving github pull request test status integration.
- Major improvements on task node page: now shows the error or warning at the top of the page.
- Fix drush aliases URI and adding files folder path alias.
- Fixing issues with default servers not respecting settings.
- Improving project settings form using collapsible fieldsets.
- Improving dynamic reloading on project creation page.
- Update to Drupal 6.37.
- GitHub settings page improvements.
- Automatic GitHub SSH key addition.
- Automatic GitHub webhook setup.
- Using GitHub API to check that SSH access has been granted.
- Added a page outputting all devshop environments and their IP addresses. Useful for hosts file, when DNS is unavailable.
- Fixing "clone" task to appear on the clone destination environment, improving user experience.
- Adding help widget for http://devshop.support.

## 0.3.1 (July 7, 2015)

- Docs overhaul: reorganization and styling.
- Passing permalink for test results to GitHub.
- Fixing accidental deletion of extra domain aliases.
- Blocking access to "webhook" and "login link" to those who lack permission.
- Fixing Views Bulk Operations version to 1.x since all 6.x releases were removed.
- Put modalframe in contrib folder.
- Officially added GPLv2 license.
- Changing vagrant box to default to ubuntu/trusty

## 0.3.0 (June 25, 2015)

- Fixing bugs in the install script for CentOS machines.
- Removed Solr from the default installer.
- Improved documentation for Install Script and overall architecture.
- Added the "DevShop CLI" with:
  - 'devshop status' command: Checks the currently installed versions.
  - 'devshop login' command: Gives user a link to login to Devmaster.
  - 'devshop upgrade' command: Walks the user through upgrading devshop to a newer version.
- Added an example module called "DevShop Extra Users" that will create extra drupal users post site installation.
- Fixed default db and web servers used for new environments.
- Added "Verify" to the available tasks in the Projects interface.
- Fixing GitHub commit status integration. It wasn't running on initial Pull Request.
- Adding DevShop tests to commit status integration. Each commit now reports back on the results of test runs and of the deployment itself.

## 0.2.2 (May 20, 2015)

- Hosting 2.4 security release. See http://community.aegirproject.org/2.4.
- Simplifying Vagrant variables.
- Fixing problem with Vagrant up preparing the devmaster files in the wrong folder.
- Adding modalframe back to the installer.

## 0.2.1 (May 5, 2015
 
- Putting back modalframe.module for now.  Can't remove it quite yet, sadly.

## 0.2.0 (May 5, 2015)

- Major improvements to documentation.
- Removing Aegir's "Welcome" page.
- Adding "Default environment settings" to projects form.
- Show the project nav on any node in the project.
- Lots of minor UI tweaks and improvements.
- Allow changing the "path to drupal" in the project create wizard.
- Allow changing the "base URL" of a project.
- Added "Default environment settings" including default environment servers.
- Improved project navigation across project components.
- Refactored DevShop Testing module to be more organized.
- Improving Testing user interface.
- Use checkboxes for deciding which behat tests to run.
- Improved task template.
- Improved test results output.
- Test Results get their own page, allowing anonymous access.
- Fixed access control and permissions for various components.
- Test results now get saved to files instead of being attached to tasks args.
- In-browser Test logs can now be 'followed'.
- Added DevShop GitHub module:
  - Allows direct interaction with GitHub API.
  - Pull request information is saved in devshop, allowing links back to the pull request.
  - Deploy and commit test status is posted back to the pull request.
- Long environment names and URLs no longer break out of the environment box.
- Hosting Solr now supports Jetty and Solr 3.x
- Ensure HEAD installs use git for deploying source code.
- Switching devmaster to match semantic versioning of devshop.

## 0.1.0 (February 7, 2015)

* Completely new, responsive user interface.
* Deploy Code, Data, or Stack.
* Pre-installed Solr.
* Run Tests with built-in behat.
* Improved documentation.
* Simplified codebase: moving drupal distibution to devmaster, everything else to devshop repo.
* Added devshop_hosting module to devmaster repo.
* Added Vagrantfile.
* Cross Platform install script.
* Added ansible playbook.
* Added behat tests.
* TravisCI integration.
* Numerous other improvements

## Previous

The changelog began with version 0.1.0 so any changes prior to that can be seen by checking the tagged releases and reading git commit messages.

Before 0.1.0, devshop releases were tagged as if it were a module: 6.x-1.x.
