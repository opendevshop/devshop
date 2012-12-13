
DevShop Hosting
===============
Drupal development infrastructure made easy.

This module provides the front-end interface needed to
deploy and manage sites using the DevShop git and features
based development workflow.


About
-----

The goals of DevShop are...

1. to simplify management of multiple environments for multiple Drupal projects.
2. to provide web-based tools that streamline the Drupal site building workflow.
3. to provide a common, open-source infrastructure for Drupal development shops.


Installation
------------
For installation instructions, see INSTALL.txt.


DevShop Projects
----------------

DevShop functionality centers around "Projects". Aegir Project nodes store a 
Git URL, the code path, the "base url", and the branches of the remote 
repository.

DevShop allows multiple platforms and sites (for dev, test, or live purposes)
to be created very easily.  Platforms can be easily created from existing
branches of your git repositories.


Creating Projects
-----------------

To create a new project, visit either the Projects page or click 
"Create Content" > "DevShop Project".

### Step 1: Git URL and project name.

Enter your project's Git URL and Project Name.

NOTE: Your project's git repo must be a complete drupal core file set.  It
should match the structure of Drupal core's git repository, and can be a clone
of http://git.drupalcode.org/project/drupal.git

### Step 2: File Path and Base URL

Enter the base path to the Project's code. Recommended is /var/aegir/projects

Enter the base URL for the project.  All Project Sites will be on a subdomain of this base URL.

### Step 3: Choose Platforms

To complete this step, the Verify Project task must finish.

On this page, you choose if you want dev, test, or live platforms and what branches each should live on.  You can also choose the branches of your git repository you wish to create platforms and sites for.






Provision Commands & Hostmaster Tasks
-------------------------------------

Once enabled, Aegir Sites will have 3 new Tasks available to them.

NOTE: Not all tasks should be run on certain sites.  It is up to YOU to decide
where and when to run these tasks.  DevShop is NOT aware of which site is live,
staging, testing, or development.  Use these commands with caution.

@TODO: More constrained tasks are being created as tasks on projects.

All tasks have specific permissions, so you can grant roles individual tasks.

1. Pull Code | drush @alias provision-devshop-pull | drush @alias pdp
  This task pulls your code, runs new updates, reverts features, and clears
  caches.  It can be used as a Deployment task, for test sites

  - Git Pull the code for your site's platform.
  - Then, all optionally:
    - Run update.php.
    - Revert all Features modules
    - Clear caches

2. Commit Features | drush @alias provision-devshop-commit | drush @alias pdc
  This task integrates with Features.module to make it very easy to recreate and
  commit your features

  - Calls drush features-update-all
  - Commits the result, with a part automated and part customized commit message.
  - (Optionally) pushes the commits.
  - (Optionally) force-reverts after a commit.

3. Sync Content | drush provision-devshop-sync @source @destination | drush pds @source @destination
  This task pulls content down from a site that has a drush alias on the current
  system. Currently any alias can be entered when Syncing content.  Eventually,
  the site will store its default "source" site.

  WARNING: This command is built as a backend command.  There are NO PROMPTS
  before the scripts will sql-drop the @destination database.  It should NEVER
  be used on a production site.  Once "DevShop Environments" is in place, we can
  prevent this command from even being called on a "live" environment.

  This task:
  - (optionally) Pulls code
  - Drops the @destination database.
  - Creates an SQL dump from @source.
  - Copies the SQL dump to the local system (if @source is a remote).
  - Imports the SQL dump into @destination database.
  - (optionally) Runs update.php.
  - (optionally) Runs features-revert-all.
  - (optionally) Clears all caches.
