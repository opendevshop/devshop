# Change Log

# 0.7.3 (December 30, 2015)

22 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.7.2...0.7.3

- Minor improvements to ajax task loader to improve performance: don't load deleted environments!
- Removing a couple of PHP notices.
- Separating node-site template to only affect sites that are in projects.
- UI Improvements: 
  - Don't show site-related links before there is a site.
  - Add "Aegir Site" and "Aegir Platform" links to the dropdown, if user has access.
  - Improving last task display: now displays text status. Much easier to tell the status, especially if you are color blind. 
  - Improved "disabled" and "deleting" environment indicators.
- Moved task icon/label/class determination to hook_load() so we don't have to do it in many places.
- Properly load clone task on target environment.
- Improve output for failed tasks, giving users buttons to take their next steps: "Retry" or "Destroy".
- Blocking clone tasks from being retried because old tasks will fail due to unversioned task arguments.
- Added "Project Messages" so we can inform the user of project wide problems (such as no deploy hooks configured.)
- Added a "release-prep.sh" and "release.sh" script to help make releases easier.

# 0.7.2 (December 23, 2015)

3 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.7.1...0.7.2

- Fixing a slew of PHP notices.
- Removing Hosting Task Jenkins from the default build. It requires composer install, and we can't run that inside of hostmaster-migrate at the moment.

# 0.7.1 (December 23, 2015)

1 commit to DevShop.
Fixed a bug in the Install command when specifying a version.

# 0.7.0 (December 23, 2015)

71 commits to DevShop: https://github.com/opendevshop/devshop/compare/0.6.0...0.7.0
20 commits to Devmaster: https://github.com/opendevshop/devmaster/compare/0.6.0...0.7.0

## DevMaster Fixes

- Fixed "Last Task" bug that was causing inconsistent environment status displays.
- Re-opening GitHub pull requests will recreate the environment.
- Fixed "Login" modal window bug that prevented users from being able to log in to all environments.
- Added "Environment Warnings" display that shows problems to the user, such as "No deploy hooks configured".
- When a major problem is detected, such as "Installation failed", "Clone failed", we now show the user a message describing what happened, and offer Retry and Destroy buttons so they can take immediate action.
- When an environment is being created, instead of saying "Verify" the first time, it says "Cloning codebase".
- Adding a VERSION.txt file to the install profile to define the project's version.  Once we go to Drupal 7 we can move this to the .info file. 

## DevShop CLI Fixes

- Fixed versioning issues! 
  - Banished the /var/aegir/.devshop-version file.  
  - Separated DevShop CLI and DevMaster versions in the status command.
  - Improved how devshop CLI interprets it's version.  If on a branch, it now specifies the SHA as well.
- Major CLI Improvements:
  - Added "self-update" command for the CLI that uses Git! Once Phar integration is complete self-update will update the phar as well.
  - Added our own Application and Command classes inspired by composer.  Moving a lot of shared code to those classes.
  - Added a sweet new logo for the CLI.
  - Set the stage for packaging devshop CLI into a PHAR file: added box.json.  We will not distribute the Phar file until we know self-update fully works.
  - Moved the executable from `devshop` to `bin/devshop`.

**NOTES:**
- After installing this release (once you have the self-update command), always run `devshop self-update` before `devshop upgrade`.  We will soon add code to enforce this by checking to see if devshop CLI is out of date before an upgrade.
- We do not remove the old .devshop-version file for you automatically, but the `devshop status` command will warn you if it still exists.  Please remove `/var/aegir/.devshop-version` manually.

# 0.6.0 (December 14, 2015)

19 commits to DevShop: https://github.com/opendevshop/devshop/compare/0.5.4...0.6.0
137 commits to Devmaster: https://github.com/opendevshop/devmaster/compare/0.5.4...0.6.0
32 commits to devshop_provision: https://github.com/opendevshop/devshop_provision/compare/0.5.0...0.6.0

## Web Developer Improvements
- Added "DevShop dotHooks" module: Add deploy hooks to your project's source code in a `.hooks` or `.hooks.yml` file.  Easily hook into any task: verify, install, deploy, test, etc.  See [docs.devshop.support](http://docs.devshop.support/en/latest/deployment-hooks/#devshop-dothooks) for more information.
- Added "DevShop Acquia" module: Use Acquia Cloud Hooks as deploy hooks in devshop. See [docs.devshop.support](http://docs.devshop.support/en/latest/deployment-hooks-acquia/) for more information.  We had partial support but now, all acquia cloud hooks are supported, and logging is much more clear.

## User Interface Improvements
- Total redesign of Tasks and logs: now we only output the logs that are pertinent to developers.
- Dynamic task logs loading for all types. Now you can sit back and watch your tasks run, with only the logs you want to see appearing.
- Renamed "Update Status" button to "Cancel Task". (TODO: Fail the task if the button is pushed.)
- Much improved deploy hooks configuration: 
  - Each environment now displays the deploy hooks that are configured, making it clear what is supposed to happen every time you deploy.
  - Project defaults are passed to all environments.
  - Block environment-specific deploy hooks.
  - Standardized deploy hook form across environment settings, deploy task, etc.
  - Automated deploy tasks respect all deploy hook types.
- Added settings form for "DevShop Public Key", in case you have to rebuild your devshop server's public key.
- Fixing problems with Aegir Download module.
- Fixed up some quirks with dynamically updating environment status.
- Created "Hosting Task Jenkins" module, allowing you to setup jenkins to run all of your tasks.  See [Hosting Tasks: Jenkins README](https://github.com/opendevshop/hosting_task_jenkins/blob/master/README.md) for more information.
- Lots of subtle design improvements.

## Internal Development Improvements
- Added hook_devshop_environment_alter(): Allow other modules to alter the environment object
- Added composer.json to devmaster and devshop_provision.
- Swapped drush_shell_exec's for Symfony Process in devshop_provision.
- Swapped provision-git-deploy for Symfony GitWrapper in devshop_provision and devshop_projects.drush.inc.  
- Added a new drush log type: devshop_command + devshop_log + devshop_ok + devshop_error.  These will output logs in a new prettier format in the front-end.  Documentation coming soon.
- Finally fully moving devshop_provision to [github](https://github.com/opendevshop/devshop_provision), Created proper tags and branches and cleaned up old ones.
- Adding a shared function for adding the deploy hooks checkboxes to all forms that need it:  devshop_environment_deploy_hooks_form()
- Cleaned out a lot of old code and comments.

# Documentation Improvements
- Added Deployment Hooks page to documentation: http://docs.devshop.support/en/latest/deployment-hooks/
- Added Automated Testing page to documentation: http://docs.devshop.support/en/latest/testing/
- Modified and cleaned up the roadmap: http://docs.devshop.support/en/latest/roadmap/
- Added hook_help() in order to improve the built in documentation!  

## New Contributors

Radim Klaška - https://github.com/radimklaska
Commit 84f9068ff4114e9f0ac9a468b8a0854f35b62e48 

Radim fixed a typo in our documentation.  Thanks, Radim!

## Site Development

- Added hook_devshop_environment_alter()
- Added "DevShop dotHooks" module: Add deploy hooks to your project's source code in a `.hooks` or `.hooks.yml` file.
- Added "DevShop Acquia" module: Use Acquia Cloud Hooks as deploy hooks in devshop.
- Total redesign of Tasks and logs: now we only output the logs that are pertinant to developers.
- Renamed "Update Status" button to "Cancel Task".
- Much improved deploy hooks configuration: 
  - Project defaults are passed to all environments.
  - Block environment-specific deploy hooks.
  - Standardized deploy hook form across environment settings, deploy task, etc.
  - Automated deploy tasks respect all deploy hook types.
- Added settings form for "DevShop Public Key", in case you have to rebuild your devshop server's public key.
- Fixing problems with Aegir Download module.
- Added composer.json to devmaster and devshop_provision.
- Swapped drush_shell_exec's for Symfony Process
- Swapped provision-git-deploy for Symfony GitWrapper
- Added Deployment Hooks page to documentation.
- Cleaned up the roadmap.


## 0.5.4 (November 7, 2015)

- Adding a "Project Info" box to the project create wizard, to show the user what they've added so far and to make it testable.
- Actually fixes #26. On project create form, the environmental default web_server was sometimes not being set.  If there is only one server, the "Step 3: Environments" form now force defaults to the only server.
- Remove the "Finish" button on the last step until all platforms are verified.
- Fixed playbook not writing a new /var/aegir/.devshop-version file on upgrade
- Slightly improved behat tests.
- Updates to travis-ci.org file.

## 0.5.3 (November 2, 2015)

- Fixing bad version number in getdevshop.com.
- Docs cleanup from @yograf (Pull Request #25)
- When calling "vagrant destroy", we now notify you that you need to delete the existing sites/devshop.local folder.
- Fix for Issue #26: "Error: cannot load node id 0 to find its context"
- Adding devshop_permissions module to (finally) provide default permissions. 
- Adding "features.module" to devshop so we can provide exported permissions.

## 0.5.2 (October 8, 2015)

- `devshop:upgrade` command can now be run interactively.
- Fixed a bug preventing saving of Aegir data on "environment settings" page, when using the site node edit form.  We moved the environment settings page back to the node/{project_nid}/edit/{environment_name} URL for now.

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
