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
For installation instructions, see http://drupal.org/project/devshop

Usage
-----

To start, you must have a Project drush alias.  Using the DevShop+Hostmaster
system will make this much easier, but if you wish to only use the backend, you
can create a project alias with provision-save, for project NAME:

  $ drush provision-save project_NAME --context_type=project --code_path=/var/aegir/projects/NAME --drupal_path= --git_url=http://git.url/to/repo.git --base_url=NAME.server.com
  
  
    $ drush provision-save project_NAME --project_name=NAME --context_type=project --code_path=/var/aegir/projects/NAME --drupal_path= --git_url=git@github.com:devudo/drupal-flat.git --base_url=NAME.server.com

Commands
--------
DevShop contains a set of features that make Drupal site building within a
version-controlled code workflow quick and easy.


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
