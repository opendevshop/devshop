# Change Log

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
