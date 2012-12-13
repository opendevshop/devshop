
DevShop Provision
=================

Drupal development infrastructure made easy.

This module provides the backend commands needed to manage projects, platforms,
and sites with DevShop Hosting.

About DevShop
-------------

The goals of DevShop are...

1. to simplify management of multiple environments for multiple Drupal projects.
2. to provide web-based tools that streamline the Drupal site building workflow.
3. to provide a common, open-source infrastructure for Drupal development shops.


Installation
------------
For installation instructions, see INSTALL.txt.


NOTE about Sync
---------------
Currently the Sync command requires a special folder in the Aegir Backups directory on 
BOTH servers:

$ mkdir ~/backups/devshop-sync

Unfortunately drush rsync cannot sync single files, as far as I know, so it syncs the 
folder.

NOTE: The reason we don't use sql-sync yet is because I had some trouble getting it
to work when the source was a remote server.  I would love some input from some
drush sql sync experts if there are any out there.


Provision Commands & Hostmaster Tasks
-------------------------------------

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
