# DevShop Control Composer Project

The web interface for DevShop is called the "DevShop Control" site. Every DevShop has one.

This code, the *DevShop Control Composer Project*, is used as a template to create
 each DevShop's Control site codebase.

This Composer project includes the [devmaster](https://drupal.org/project/devmaster) Drupal install profile and all 
other required libraries.
 
This project gets installed in every DevShop to `/var/aegir/devshop-control-1.x`, with the [opendevshop.devmaster Ansible role](../../../roles/opendevshop.devmaster/tasks/install-devmaster.yml) 

Every *DevShop Server* can then extend their *DevShop Control Site* using `composer require`, 
and can add their custom code to a private git repository to maintain customizations over time.

DevShop Control sites can now be updated in place using git and `composer`. See [Updating your DevShop Control Site](#updating-your-devshop-control-site) below.

## Customizing your DevShop Control Site

You can customize your DevShop Control site in place, but it is recommended to
 set up a Git repo to store your changes:
 
1. Create a git repository on your favorite git host:

      - https://gitlab.com/projects/new
      - https://github.com/new

2. Clone it and `cd` into it:

        git clone git@git.example.com:org/devshop.example.com.git
        cd devshop.example.com

3. Run the composer command to create a new project using this repo as a template:

        composer create-project devshop/control-template:@dev

    With no directory argument, the project will be built in the current directory.

4. Add to git and push.

        git add -A
        git commit -m 'Initial Commit!'
        git push origin master

5. Install the repo in your DevShop:

    When installing DevShop, set the Ansible variable `devshop_control_git_remote` to your new git repository. 

    See [roles/opendevshop.devmaster/defaults/main.yml](https://github.com/opendevshop/devshop/tree/1.x/roles/opendevshop.devmaster/defaults/main.yml) to see the default variable.
    
        devshop_control_git_remote: 'git@git.example.com:org/devshop.example.com.git'
        devshop_control_git_docroot: web
        devshop_control_git_reference: "1.0.0"

    If you wish to swap in your own install profile, set the Ansible variable `devshop_install_profile`:
    
        devshop_install_profile: devmaster

    @TODO: Add `--control-git-remote` option to `install.sh`.

## Updating your DevShop Control Site

This project is still in an alpha state. Upgrades will be handled with composer 
but the specific behavior of `composer update` can vary. 

These instructions are a DRAFT. We will create a single command to update properly
and will re-implement upgrade tests. 

1. Update Drupal core first:

         composer update drupal/drupal --with-dependencies

    If using git, commit the results.

3. Update the `devshop/devmaster` dependency:

         composer update devshop/devmaster --with-dependencies

4. Update the rest of the project:

         composer update devshop/devmaster --with-dependencies

See the [`drupal-composer/drupal-project` composer project for more information](https://github.com/drupal-composer/drupal-project#updating-drupal-core).

## Development

This is a mini repo, split from the [DevShop Mega repo](https://github.com/opendevshop/devshop/tree/1.x/src/DevShop/Components/DevShopControlTemplate).

Please submit pull requests and issues there.

Thanks!
