# Change Log

## 1.0.0-rc1 (%)

% Commits to Devmaster https://github.com/opendevshop/devmaster/compare/1.0.0-beta10...1.x

### Updates 

- Update Drupal Core to 7.54.
- Update Drupal Contrib.
- Update Aegir Hosting modules to 3.12.0. Release notes available at: http://docs.aegirproject.org/en/3.x/release-notes/3.12/  (Please read, MANY huge improvements.)

### New Features

- Allow environment installation by multiple methods: Install profile, clone other environment, Import from SQL Database, or leave an Empty Database!
- Add Hosting HTTPS to allow automated certificate creation and renewal from LetsEncrypt.org.
- For Pull Request environments, add option to reinstall site on every git-push! This respects the "install method", allowing re-install of profile, re-sync from production, or import from external database!
- Added ability to "Change Site Domain Name" by using the "migrate" task.
- Configure a "Environment Domain Name Pattern" instead of "Base Url". Define your environment's URLs with a pattern like "@project.@environment.@hostname" or "@environment.domain.com".
- Make all URLs in Behat console output logs clickable. Useful in test results.
- Bigger, Bolder empty projects page.
- Plenty of UI enhancements:
    - Add "Install Method" and date to environment template.
    - Add Site Slogan back into page template.
    - Improve detection and messaging of environment state.

### Aegir improvements in 3.12.0

- Switch to using Semantic Versioning!  Aegir tag 7.x-3.120 is for Aegir 3.12.0
- Preparing Aegir for Docker hosting by allowing more open connections to Database servers: https://www.drupal.org/node/2794915
- Allow hostmaster install profile to be set in Debian package variable: https://www.drupal.org/node/2886587.  This will allow DevShop devmaster to start using the aegir hostmaster debian package.
- Added 'install_method' property to Aegir sites. If set to "profile", the default behavior (site install) executes. Set to something else to skip this and to flag it for something else later: https://www.drupal.org/node/2754069
- Add option to "provision-install" for "--force-reinstall". This new option will quickly delete the site and install it again using the original install_method. https://www.drupal.org/node/2836185

### Organization

- Moved Documentation to a dedicated repository: https://github.com/opendevshop/documentation
- Pushed devmaster back into drupal.org and refactored makefiles to make it more of a true distribution. See https://www.drupal.org/project/devmaster.
- Added a separate Makefile in YML for development: ensures all projects are downloaded with git.  (devmaster.development.make.yml)

### Subsystem Improvements

- Moved git handling to Aegir's Hosting Git Module! One more step towards moving devshop into Aegir. Platforms are now saved using the git ref from the project.
- Added a docker-compose.yml file for launching in Docker. Tuned for local development but can be adapted for production use.
- Added geerlingguy.composer Ansible role.
- Removed the last remnants of our custom Ansible role files: This MOTD template never worked, anyway! https://github.com/opendevshop/devshop/blob/376a74f9db5d154fad05d6083f0f402ac0f19fba/templates/motd.j2

### Testing Improvements

- Standardized on using `robo` commands for local dev and Travis testing.
- Removed old `tests/docker-launch.sh` and `tests/docker-destroy.sh` scripts, now that we have robo.
- Improved Travis CI testing: Now installs using Docker, and with Ansible, on both CentOS and Ubuntu. Upgrade is tested as well.
- Added `--name` option to `devshop devmaster:test` command to pass to bin/behat --name.
- Added `behat-path` option to `devshop devmaster:test` command to allow customizing which set of tests to run.
- Added `--makefile` option to `devshop upgrade` command to allow overriding the desired makefile.
- Moved Behat tests for DevShop to Devmaster, since most (all?) of the code being tested is actually in that repository. Makes changing tests to adapt to changing functionality much easier if its in the same repository.
- Removed custom Dockerfiles used for testing.
- On failure, echo last page source and watchdog logs.

### DevShop Development Tools Improvements
- Added a Robofile.php for easy launching and development. Install the Robo CLI and `robo up` to get a running devshop on Docker. See http://robo.li/ for more information.
- Deprecated the Vagrant based development environment, moving it into a subfolder.

### Other Improvements

- Fixed numerous bugs with Hosting Logs. Now you can easily make error logs available to uses, if needed.
- Fixed DevShop Deploy, Aegir Commit, DevShop Acquia, DevShop Dothooks, and Aegir Download module to work with repo_path instead of repo_root.
- Change the hook named `hook_devshop_environment_actions()` to `hook_devshop_environment_menu()`.f
- Fix 2 problems with BitBucket forms and webhooks. From new contributor @josebc! https://github.com/opendevshop/devmaster/pull/73
- Fix bug where "Import/Export Config" tasks would show for Drupal 7 sites. 
- Adding Features Update and Features Revert to Environment Menu.
- Temporarily removing "Fork Environment" feature, we will get it into Hosting Git in the next RC.
- In project drush aliases, just use 'parent' property to make our @project.env aliases work exactly like provision's.
-  Renamed devshop_drush_process() to devshop_process();
- Save environment settings into provision context data.
- Fix bug preventing branch names from having "/" characters.
- Removing custom drush command `devshop-install`. Now works with `hostmaster-install`.
- Add more details to automatically generated behat.yml files.
- Adding `--strict` to `bin/behat` command so that missing steps fail the process.
- Remove DevShop Deploy queue since Hosting Git Platforms now handles that.
- Enable fix_permissions and fix_ownership modules by default for better file management.
- Fix problem with cloning sites that used custom profiles.
- Remove loading of tons of git info from the command line for a significant performance boost.
- Removing old unused devshop-create and devshop-commit tasks.

# George & Maxwell Pugh (September 20, 2016)

- 6lbs 1oz & 6lbs 2oz
- The Mount Sinai Hospital NYC
- Significantly increases the amount of awwwww.

# 1.0.0-beta10 (July 7, 2016)

  28 commits to Devmaster: https://github.com/opendevshop/devmaster/compare/1.0.0-beta9...1.x

- Added "composer install" deploy hook! You can now configure `composer install` to run on deployment. Works with composer.json in the repo root or the drupal root.
- Improvements & bug fixes to the "DevShop Remotes" module.
- Added a "Retry" button to an environment that failed a "clone" task.
- Added "Environment" to the labels for "Environment Dashboard" and "Environment Settings" links.
- Added a message if the user has a project in the create project wizard.
- Added all of bootstrap, including fonts to boots theme, allowing fully offline use.
- Moved the Cancel button in projects creation wizard to the right side of the page.
- Added icons to the "Next" button, "Add Environment" and "Finish" buttons in the create project wizard.
- Renamed the "Finish" button to "Create Project & Environments".
- Update to Drupal 7.50.

# 1.0.0-beta9 (June 28, 2016)

 11 commits to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-beta8...1.0.0-beta9

 12 commits to Devmaster: https://github.com/opendevshop/devmaster/compare/1.0.0-beta8...1.0.0-beta9

- Bumped Drupal to 7.44, Hosting to 3.6, and Views to 3.14.
- Added '--force' to 'git submodule' updates on deploy.
- Adding local copies of bootstrap js and css so devshop can work offline.
- Replaced "\n" with actual new lines in ASCII output.
- Improved creation wizard by loading all errors into a modal window.
- Fixed bad drush project alias file creation.
- Fixed the missing 'Cancel' button on tasks.
- Added a getEnvironment() method to the project context.
- Add an argument to devshop_drush_process() to be able to skip logging the output.
- Verify Project is triggered after environment create to ensure metadata is present.
- Decided to remove the code that skips running deploy hooks if there are "no changes detected".
- DevMaster Playbook fixes:
  - Fixed the automatic setting of the aegir public ssh key variable.
  - Adding back SSH key privacy settings, git config, and drush cache clearing.
  - Fixed typos in yml tasks.
  - Fixed a terrible bug that broke installation: Clear Drush Caches!
- Bumped up default vagrant VM memory to 4GB.

# 1.0.0-beta8 (June 15, 2016)

1 commit to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-beta7...1.0.0-beta8

29 commit to DevMaster: https://github.com/opendevshop/devmaster/compare/1.0.0-beta7...1.0.0-beta8

- Fixed script to launch devshop in docker containers for development.
- Fixed numerous problems with DevShop Testing module preventing fully automated testing.
- Added an example Behat Drupal tests folder: https://github.com/opendevshop/devmaster/tree/1.x/modules/devshop/devshop_testing/tests_example
- Fixed bug preventing new tasks from loading into the page. No more reloading the page.
- Added dynamic timestamps using the timeago plugin so it's always updating timestamps.
- Fixed a number of UI bugs.
- Fixed broken Project Delete link.
- Added "--force" to "git submodule update" command.
- We created a commercial! https://www.youtube.com/watch?v=L3G2BxDkgPk

# 1.0.0-beta7 (June 14, 2016)

- Hotfix: Update hook to enable DevShop testing failes. Removed it for now.

# 1.0.0-beta6 (June 13, 2016)

- Hotfix: Project settings weren't saving properly.

# 1.0.0-beta5 (June 13, 2016)

1 commits to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-beta4...1.0.0-beta5

40 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/1.0.0-beta4...1.0.0-beta5

- Cleaned up environment widget, consolidating links and hooks output.
- Major cleanup of environment settings form by putting it into vertical tabs!
- Minor cleanups of project settings form.
- Cleanup of the Projects page.
- Added Aegir Update module! One click drupal core updates. (Experimental)
- Enable DevShop Testing by default! Everyone should test.
- Changed "Live Environment" to "Primary Environment".
- Fixed Bugs in provision-commit.
- Cleaned up "hosting features" list: making sure optional features aren't marked "Enabled".
- Fixed links to task logs.
- Fixed link to Edit Domains.
- Fixed the matchHeight plugin to make the environment grid even.
- Allow logos to be added.
- Cleaned up SSH warnings output when creating a project fails.
- Fixed bug preventing verification of projects after saving a site.
- Added better help text on project creation form.
- Fixed listing tags with ^{} characters in them.
- Added devshop remotes to hosting features.
- Made git repo field 1024 characters long, making it compatible with Pantheon.

# 1.0.0-beta4 (June 11, 2016)

14 commits to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-beta3...1.0.0-beta4

18 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/1.0.0-beta3...1.0.0-beta4

7 commits to DevMaster Ansible Role: https://github.com/opendevshop/ansible-role-devmaster/compare/1.0.2...1.1.0

- Improvements to server node template.
- Added "devshop_devmaster_email" as an ansible variable so it can be configured at install time.
- Upgraded to drush 8.1.2
- Moved mysql_root_password to "defaults" so it can be overridden.
- Fixing a bug that broke environment database server selection when multiple database servers exist.
- Removing the old "allow deploying data from drush aliases" setting from project settings. Now you just have to enable DevShop Remotes.
- Removing menu settings for project node form.
- Improved the "Create Sites" step in the project create wizard. Now properly alerts you to a failed git clone, and added a retry button.
- Improved help text in step 1 of project creation.
- Removed broken link to files browser.

## New Contributors

  Two new contributors helped out with documentation and .gitignore changes:

  - Travis Christopher https://github.com/arttus
  - Tommy Cox: https://github.com/tommycox

# 1.0.0-beta3 (May 24, 2016)

6 commits to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-beta2...1.x
234 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/1.0.0-beta2...1.x

- Added "DevShop Remotes" module: Easily add remote drush aliases to use as database sources.
- Locked the devshop installer ansible roles to specific verions.
- Created a new "Process" service for aegir servers for easier command running:
- Fixed javascript reloading on completed tasks. You can now copy and paste out of logs!
- Many commits in this release came from DevShop Cloud, now known as Aegir Cloud. We've removed the codebase from core devshop until a final release is ready. See http://drupal.org/project/aegir_cloud more info.


# 1.0.0-beta2 (May 9, 2016)

Released at DrupalCon New Orleans!

64 Commits to DevShop: https://github.com/opendevshop/devshop/compare/release-1.0.0-beta1...1.x
25 Commits to DevMaster: https://github.com/opendevshop/devmaster/compare/release-1.0.0-beta1...1.x

- Release of official Ansible Galaxy Roles: http://galaxy.ansible.com/opendevshop 
- Added beta of DevShop BitBucket integration, allowing webhooks and pull request environments for BitBucket repos.
- UI Improvements: Vertical Tabs in project settings!
- New Contributor: @tommycox.  Thanks for cleaning up images in the Documentation!

# 1.0.0-beta1 (April 20, 2016)

104 Commits to DevShop: https://github.com/opendevshop/devshop/compare/release-1.0.0-alpha4...1.x
67 Commits to DevMaster: https://github.com/opendevshop/devmaster/compare/release-1.0.0-alpha4...1.x

## Ansible Playbooks

- Rewrote ansible installer using @geerlingguy's Ansible Playbooks.
- Created aegir.user, aegir.apache, aegir.nginx, and aegir.devmaster roles to finalize DevMaster installation.
  - https://github.com/opendevshop/ansible-role-aegir-user
  - https://github.com/opendevshop/ansible-role-aegir-apache
  - https://github.com/opendevshop/ansible-role-aegir-nginx
  - https://github.com/opendevshop/ansible-role-devmaster
- Improved install.sh script to work better on RedHat Enterprise Linux.
- Developed "Aegir Ansible" project, to be included in next release.  Allows Aegir to act as an Ansible Inventory, making setting up remote servers and adding additional playbooks and roles a breeze.

## New Features

- Git Submodules! Deploy tasks now run `git submodule update --init --recursive`, so if you want to keep certain modules in different git repos, devshop now makes that easy.
- Config Export and Import! DevShop & Aegir now give you a button to press to export your Drupal 8 config to files.  When you combine this with the Aegir Commit modules, you can Site Build, Export to disk, and commit to git without ever leaving your browser.
- Upgrade Drupal! Click the "Upgrade Drupal" button to run "drush pm-update" to get your core and contrib in line.
- Grouped Drush aliases! Each project now writes it's own drush aliases file, making it much easier to target an environment: 'drush @project.environment uli`

## D6 -> D7 Upgrade Fixes

- Fixed user redirection when running tasks. If fired from project dashboard, user is returned to project dashboard. If fired from environment dashboard, user is returned to environment dashboard.
- Fixed dynamic task status loading. Just sit back and watch tasks start and stop.
- Cleaned up styles for tasks widget, especially on the environments dashboard.
- A few missing links to the new hostmaster URL 'hosting_confirm/1' instead of node/1'
- Removed old unneeded CSS and JS.
- Fixed the deploy queue: DevShop can once again continuously update your environments that are tracking git branches without a webhook.


# 1.0.0-alpha4 (March 23, 2016)

23 Commits to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-alpha3...1.x
1 Commit to Devmaster: https://github.com/opendevshop/devmaster/compare/1.0.0-alpha3...1.x

- Fixed a missing redirect from a newly created project node to the create project wizard.
- Fixed broken PHP 5.5 installation on RedHat family.
- Improved devshop login and devshop status commands: they will now attempt to switch to aegir user.

# 1.0.0-alpha3 (March 15, 2016)

9 commits to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-alpha2...1.x
89 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/1.0.0-alpha2...1.x

- Improved installation documentation.
- Display "diverged" for environments with diverged git repos.
- Added "devshop_stats" module to allow us to track usage using drupal.org.
- Fixing the over-obfuscated git url.
- Changing Vagrantfile to use hostname "local.devshop.site"
- Removing dependencies on Provision so devmaster can be installed as a standalone site.
- Added "Module Filter" module.
- Updating Aegir Commit, Aegir Download, and Aegir Features modules to 7.x.
- Many 7.x upgrade theming fixes.
- Adding "Default Install Profile" field to project settings form: you can now change the install profile that will be used for new environments.
- Fixed GitHub Integration settings in Project Settings form.
- Improved Pull Request settings dropdown: Now you can select "install" or a specific environment to clone (not forced to use the "live" environment.)
- Fixed bugs preventing Pull Request environments from loading data.
- Added "Commit Code" link to environment settings dropdown.

## 0.9.0 (March 15, 2016)

12 commits to DevShop: https://github.com/opendevshop/devshop/compare/0.8.1...0.x
x commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.8.1...0.x

- Upgrade to Drupal 6.38!  Drupal 6.x has reached End of Life but we will support it until the 1.0.0 release of DevShop.
- Last minor version in the 0.x branch.  
- Backported improvements from 1.0.0-alpha3.

## 1.0.0-alpha2 (March 10, 2016)

5 commits to DevShop: https://github.com/opendevshop/devshop/compare/1.0.0-alpha1...1.x
37 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/1.0.0-alpha1...1.x

- Adding devshop_stats to devmaster.make. It was somehow missed in the last release.
- Changed hostname of VM to local.devshop.site
- Moving code around to make devmaster able to install without provision.
- Fixing bad password obfuscation for SSH URLs
- Bumping Aegir Commit and Aegir Features to 7.x
- Make "default install profile" configurable in project settings.
- Fix bug causing loss of "install profile" in project settings.


## 1.0.0-alpha1 (February 26, 2016)

First Alpha for 1.0.0!
 
257 commits to DevShop: https://github.com/opendevshop/devshop/compare/0.8.1...1.x
725 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.8.1...1.x
 
### Hosting Drupal 8!

- Upgraded devshop to use Aegir 3.x: Drupal 7 front-end, Drush 8, able to host Drupal 8.
- There in an immense amount of work that went into this release.  We will be putting together a master list for the 
  first beta.
- Mostly, this was a direct port. Most improvements done in this branch were already ported to 0.x branch and were released in the 0.x family.

More coming soon... 

## 0.8.1 (February 17, 2016)

HotFix release.

7 commits to DevShop: https://github.com/opendevshop/devshop/compare/0.8.0...0.8.1
5 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.8.0...0.8.1

- Fixed install.sh bug that prevented supervisor jobs from being setup.
- Added more screenshots to the documentation.
- If Git URLs have passwords, scrub them before showing in the web browser.  (Thanks @llwp!)
- A small typo fix in the README.md (Thanks, @RealLukeMartin!)

## 0.8.0 (February 10, 2016)

313 commits to DevShop: https://github.com/opendevshop/devshop/compare/0.7.4...0.8.0
159 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.7.4...0.8.0

### Newly Redesigned Tasks

- *Recreate Features* button allows you to update some or all of your Features modules in one click. Yes, this is an old feature, but it needed some dusting off. We've decoupled the "recreate" process from the "commit" process, making it much easier to avoid mistakes.
- *Download Modules* button allows you download and commit modules to git with the press of a button, or you can save your commit for the...
- *Commit* button, in the *Git Information* panel.  This task allows you to easily fill in a commit message and commit some or all of your changes. The name and user in DevMaster is automatically passed to git, so keep your devshop users accurate and you will have accurate git history.

These new features are not enabled by default, yet.  To check them out, please enable `aegir_features`, `aegir_download`, and `aegir_commit`.

### UI Improvements

- *Git Information* tells you when you have untracked, modified, added or deleted files. It will indicate if the repo is ahead or behind. Click for a modal window showing you the current git status, last commit, and full git diff for every environment.
- Git reference displayed on environments is now loaded in *realtime*. 
- *Situational Indicators* now tell you more about what is happening with your environments, especially when something failed.
- Situational buttons now appear on failed tasks or disabled environments giving you easy access to your options, such as 'View Logs & Retry', or 'Destroy Environment'.
- Improve `devshop status` command, it now fully bootstraps the devmaster site to ensure it is working and properly exits with a non-zero exit code if something fails.
- Output devmaster drupal version and site URL in `devshop status` command.
- Use `devshop status` and `devshop login` at end of install script, making it much more friendly.
- Moved all commands to use devshop_drush_process() and our new logging system, making all task logs beautiful.
- Added a `theme('devshop_ascii', $string)` function for easy beautiful ascii color in browser.
- Fixed the problem of GitHub Pull Request Environments often causing the "Clone" task to fail, which would prevent the environment from being created. In addition, "Deploy" task would not run on Cloned sites.  Now, new PR environments are exact clones of live, on the same git ref. After that, a new "Deploy" task is created that runs all appropriate deploy hooks.
- Clones of locked environments are automatically unlocked, since they are assumed to be destructible. Be sure to check your environment settings if you need to lock environments.
- Removing our special devshop_tests drupal page, and altering the menu to allow access to test tasks to users with the permission "access test logs". Now github links to the correct place to view tests in action!
- Fixed the funky "Logs" page for environments, and removed devshop_logs.module altogether!
- Added a [Tour](http://docs.devshop.support/en/latest/#tour) section to the documentation with screenshots.


### System & DevShop Development Features

- NGINX! You can now install NGINX + PHP-FPM on the devmaster server using the install script option `--server-webserver=nginx`.
- Full test suite running on Travis-ci.org, on both repositories, for both NGINX and Apache, Ubuntu 14.04 and CentOS7!.  See https://travis-ci.org/opendevshop/devmaster and https://travis-ci.org/opendevshop/devshop for more info.
- Refactored travis tests and install scripts to allow testing of Pull Requests on devmaster repo.
- DevShop Behat tests now run through entire project creation, even testing the environment was installed properly.
- Created `docker-development-environment.sh` to launch a DevShop development environment in docker!  See `docker-destroy-devshop.sh` to remove the containers.
- Adding a 'devshop devmaster:test` command to the CLI for easy Behat testing of the front-end.
- Added command line options to `install.sh` script: 
  - `--server-webserver` can be `apache` or `nginx`.  
  - `--hostname` will pass a desired hostname to ansible, which will change the hostname on the system.
  - `--makefile` passes a different "build-devmaster.make" file, allowing for pull request testing.
  - `--aegir_user_uid` will pass to ansible variables, setting the aegir user to use the desired UID.


### New Contributors!

We're so excited to welcome Andrew Rosborough (@arosboro) from DropForgeLabs.com, and Daniel Hesoyam (@Hesoyam) to OpenDevShop as our newest contributors!

Andrew did immense work on tests, NGINX, and our 1.x branch upgrading devmaster to Drupal 7.

Daniel submitted a small patch fixing a warning in old versions of Vagrant.

Welcome!

## 0.7.4 (January 15, 2016)

1 commits to DevShop: https://github.com/opendevshop/devmaster/compare/0.7.3...0.7.4
15 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.7.3...0.7.4

- Fixed bug that blocked non-uid-1 users from cloning and forking environments.
- Now runs apt-get update before installing git.
- Added "Run Tests" to Project Settings "Default Deploy Hooks".
- Improvements to the way devshop_get_tasks() works, improving dynamic task loading performance.
- Major improvements to Environment status user interface. It now clearly states to the user what an environment is doing: "Creating environment", "Cloning Environment", "Deleting Environment", "Disabling Environment", "Clone failed", "Delete Failed", Etc.  Added separate "Site Destroy" and "Platform destroy" indicators.
- Major improvements to GitHub pull request environment interface. Now clearly shows PR number and environment name. Shows Pull request title as well.

## 0.7.3 (December 30, 2015)

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

## 0.7.2 (December 23, 2015)

3 commits to DevMaster: https://github.com/opendevshop/devmaster/compare/0.7.1...0.7.2

- Fixing a slew of PHP notices.
- Removing Hosting Task Jenkins from the default build. It requires composer install, and we can't run that inside of hostmaster-migrate at the moment.

## 0.7.1 (December 23, 2015)

1 commit to DevShop.
Fixed a bug in the Install command when specifying a version.

## 0.7.0 (December 23, 2015)

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

## 0.6.0 (December 14, 2015)

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

## Legacy

The changelog began with version 0.1.0 so any changes prior to that can be seen by checking the tagged releases and reading git commit messages.

Before 0.1.0, devshop releases were tagged as if it were a module: 6.x-1.x.

There was a long history of addon modules for aegir before devshop. 

### Hosting & Provision Logs (December 20, 2012)

https://www.drupal.org/project/hosting_logs
https://www.drupal.org/project/provision_logs

Improved error log handling by setting Apache logs to an aegir-readable location, and making
them accessible to users through the devshop UI and a URL on the site.

### Hosting & Provision Solr (September 12, 2012)

https://www.drupal.org/project/hosting_solr
https://www.drupal.org/project/provision_solr

This module allows Solr servers to be added to Aegir. Once you have a server, you can give an Aegir Site a Solr database as easily as choosing it's DB Server.

### DevMaster (September 9, 2012)

https://www.drupal.org/project/devmaster

It wasn't until September of 2012 that we decided we needed a dedicated install profile, 
separate from Aegir's Hostmaster.

### DevShop Hosting (March 1, 2012)

https://www.drupal.org/project/devshop_hosting

Provided the front-end interface for DevShop before it was merged into the DevMaster install profile project.

### DevShop Provision (March 1, 2012)

https://www.drupal.org/project/devshop_provision

Provision drush commands for devshop. Still in use until 1.x release!

### Hosting Features (February 25, 2012)

https://www.drupal.org/project/hosting_features

Allows Hostmaster users with the right permission to trigger a "Update & Commit Features" task. This re-creates all of a site's features (as in features.module features) and commits (and pushes) them

### Provision Git Features (February 25, 2012) 

https://www.drupal.org/project/provision_git_features

The backend commands that Hosting Features needs to work.

### Provision Git (February 21, 2012)

https://www.drupal.org/project/provision_git

Provides Provision with simple git commands.
