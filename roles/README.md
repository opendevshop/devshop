# Ansible Galaxy Roles

This folder contains roles distributed on https://galaxy.ansible.com,

To improve stability and remove the need for extra build steps, the roles are committed directly to this repo.

This is modeled on the DrupalVM repository. See https://github.com/geerlingguy/drupal-vm/tree/master/provisioning

## Adding and Updating Galaxy roles

To simplify role management, we've come up with a model for committing roles to code.

To update roles to the latest versions, just run `composer update` and commit
the result.

To install a new role, add it to `roles/roles.yml`, then run `composer update` 
to install the roles.

DevShop's `composer.json` file has a `post-update-cmd` hook that calls
`ansible-galaxy install` with the `--force` option.  

## Forking Roles

You can easily fork a role and include your version in this repository. 

Change the entry in `roles.yml` to point to your repository instead:

    # from GitHub, overriding the name and specifying a specific tag
    - src: https://github.com/bennojoy/nginx
      version: master
      name: nginx_role
