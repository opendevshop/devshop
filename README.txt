
DevShop Hosting
===============
Drupal development infrastructure made easy.

This module provides the front-end interface needed to
deploy and manage sites using the DevShop git and features
based development workflow.

About DevShop
-------------
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
DevShop contains a set of features that make Drupal site building within a
version-controlled code workflow quick and easy.

All tasks are run on project nodes, and all tasks have specific permissions, so you can grant roles the permission to fire specific tasks.

1. Pull Code
  $ drush @project_NAME provision-devshop-pull ENVIRONMENT
   
  This task runs on the dev platform for this project. It runs git pull, and
  optionally runs new updates, reverts features, and clears caches.  It is used
  to keep the dev server up to date on every commit via the devshop_pull module,
  and can be used as the deployment task.

  - Git Pull the code for your site's platform.
  - Then, all optionally:
    - Run update.php.
    - Revert all Features modules
    - Clear caches

2. Commit Features
  $ drush @project_NAME provision-devshop-commit ENVIRONMENT --message="My Commit"
  
  This task integrates with Features.module to make it very easy to commit
  your changes to your features.

  - Calls drush features-update-all
  - Commits the result, with a part automated and part customized commit message.
  - (Optionally) pushes the commits.
  - (Optionally) force-reverts after a commit.

3. Sync Content
  $ drush @project_NAME provision-devshop-sync SOURCE_ENVIRONMENT DESTINATION_ENVIRONMENT
  
  This task makes it easy to syncronize the database and filesdown from other
  environments within the project.

  WARNING: This will DESTROY the destination site's database!

  This task:
  - (optionally) Pulls code
  - Drops the @destination database.
  - Creates an SQL dump from @source.
  - Copies the SQL dump to the local system (if @source is a remote).
  - Imports the SQL dump into @destination database.
  - (optionally) Runs update.php.
  - (optionally) Runs features-revert-all.
  - (optionally) Clears all caches.
