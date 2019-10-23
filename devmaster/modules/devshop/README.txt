
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
