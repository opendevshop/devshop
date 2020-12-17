# DevShop Control

The web interface for DevShop is called the "DevShop Control" site. Every DevShop has one.

This code, the *DevShop Control Composer Project*, is the codebase for the Devshop Control site.

DevShop Control includes the [devmaster](https://drupal.org/project/devmaster) Drupal install profile and all 
other required modules and libraries via Composer.

This project was developed from the `7.x` branch of the [drupal-composer/drupal-project](https://github.com/drupal-composer/drupal-project#updating-drupal-core).


## Source Code

This project is included in the main DevShop source code in the folder [./src/DevShop/Component/ControlProject](src/DevShop/Component/ControlProject) which is typically installed to `/usr/share/devshop`.

A standard DevShop install will set up the DevShopControl site using that folder. 

If you want to extend your DevShop Control site, you can copy the files from that folder, or use the `composer create-project` command to scaffold up a new composer stack.

Once you have a custom codebase, you can put the code into a git repository.

## Creating a custom DevShop Control Site.

Creating a custom DevShop Control is just like any other Drupal site: Use Composer and Git.


### Part 1: Create the codebase.

1. Create a git repository on your favorite git host:

      - https://gitlab.com/projects/new
      - https://github.com/new

2. Clone it and `cd` into it:

        git clone git@git.example.com:org/devshop.example.com.git
        cd devshop.example.com

3. Run the composer command to create a new project using this repo as a template:

        composer create-project devshop/control-project:@dev

    With no directory argument, the project will be built in the current directory.

4. Add to git and push.

        git add -A
        git commit -m 'First Commit!'
        git branch -M main
        git push -u origin main

### Part 2: Installing the Codebase

When installing DevShop, you can change what git repo is used for installing the DevShop Control site via Ansible Variables.

Set the following variables in your Ansible inventory. There are many places Ansible variables can go, such as `/etc/ansible/hosts` or a file in `/etc/ansible/host_vars`. 

See [roles/opendevshop.devmaster/defaults/main.yml](./roles/opendevshop.devmaster/defaults/main.yml) to see the default variable values.
 
     devshop_control_git_remote: 'git@git.example.com:org/devshop.example.com.git'
     devshop_control_git_docroot: web
     devshop_control_git_reference: "main"
     devshop_control_git_root: "/var/aegir/devshop.example.com"

 If you wish to run your own install profile during the Ansible install, set the variable `devshop_install_profile`:
 
     devshop_install_profile: devmaster

## Development

This is a mini repo, split from the [DevShop Mega repo](https://github.com/opendevshop/devshop/tree/1.x/src/DevShop/Component/ControlProject).

Please submit pull requests and issues there.

Thanks!
