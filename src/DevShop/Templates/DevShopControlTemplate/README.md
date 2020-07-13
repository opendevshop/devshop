# DevShop Devmaster Composer Project

This is the Composer-based Devmaster platform template. Drupal 7 packages are loaded 
from the official https://packages.drupal.org/7

This repository is cloned directly by default for the Devmaster platform. You can also use it to create a custom
Devmaster instance using the `composer create-project` command.

## Customizing your Devmaster Instance

You can set up a Git repo to hold the code for your Devmaster site just like any other Drupal site:

1. Create a git repository.
2. Clone it and `cd` into it.
3. Run the composer command to create a new project using this repo as a template:

        composer create-project devshop/control-template:@dev

  With no directory argument, the project will be built in the current directory.

4. Add to git and push.
5. When installing DevShop, set the Ansible variable `devmaster_git_repo` to your new git repository. 

  See `./roles/opendevshop.devmaster/defaults/main.yml` to see the default variable.
    
        devmaster_git_repo: 'https://github.com/devshop-packages/devshop-control-template'

  @TODO: Once an install.sh option is created, add to this documentation.
