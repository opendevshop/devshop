# Ansible Role: Drupal

[![CI](https://github.com/geerlingguy/ansible-role-drupal/workflows/CI/badge.svg?event=push)](https://github.com/geerlingguy/ansible-role-drupal/actions?query=workflow%3ACI)

Builds and installs [Drupal](https://drupal.org/), an open source content management platform.

## Requirements

Drupal is a PHP-based application that is meant to run behind a typical LAMP/LEMP/LEPP/etc. stack, so you'll need at least the following:

  - Apache or Nginx (Recommended: `geerlingguy.apache` or `geerlingguy.nginx`)
  - MySQL or similar Database server (Recommended: `geerlingguy.mysql` or `geerlingguy.postgresql`)
  - PHP (Recommended: `geerlingguy.php` along with other PHP-related roles like `php-mysql`).

Drush is not an absolute requirement, but it's handy to have, and also required if you use this role to Install a Drupal site (`drupal_install_site: true`). You can use `geerlingguy.drush` to install Drush.

Git is not an absolute requirement, but is required if you're deploying from a Git repository (e.g. `drupal_deploy: true`). You can use `geerlingguy.git` to install Git.

## Role Variables

Available variables are listed below, along with default values (see `defaults/main.yml`):

### Deploy an existing project with Git

    drupal_deploy: false
    drupal_deploy_repo: ""
    drupal_deploy_version: master
    drupal_deploy_update: true
    drupal_deploy_dir: "/var/www/drupal"
    drupal_deploy_accept_hostkey: false

Set `drupal_deploy` to `true` and `drupal_build_composer*` to `false` if you would like to deploy Drupal to your server from an existing Git repository. The other options all apply to the Git checkout operation:

  - `repo`: Git repository address
  - `version`: can be a branch, tag, or commit hash
  - `update`: whether the repository should be updated to the latest commit, if `version` is a branch
  - `dir`: The directory into which the repository will be checked out
  - `accept_hostkey`: Whether to automatically accept the Git server's hostkey on the first connection.

You can also control whether a `composer install` is run after the git clone is finished using the following variable:

    drupal_deploy_composer_install: true

### Build a project from a Drush Make file

    drupal_build_makefile: false
    drush_makefile_path: "/path/to/drupal.make.yml"
    drush_make_options: "--no-gitinfofile"

Set this to `true` and `drupal_build_composer*` to `false` if you would like to build a Drupal make file with Drush.

### Build a project from a Composer file

    drupal_build_composer: false
    drupal_composer_path: "/path/to/drupal.composer.json"
    drupal_composer_install_dir: "/var/www/drupal"
    drupal_composer_no_dev: true
    drupal_composer_dependencies:
      - "drush/drush:^10.1"

Set `drupal_build_makefile` to `false` and this to `true` if you are using a Composer-based site deployment strategy. The other options should be relatively straightforward.

    drupal_composer_bin_dir: "vendor/bin"

If you set the `bin-dir` in your project's `composer.json` file to a value other than `vendor/bin`, override this variable with the same directory path.

### Create a new project using `composer create-project` (Composer)

    drupal_build_composer_project: true
    drupal_composer_project_package: "drupal/recommended-project:^9@dev"
    drupal_composer_project_options: "--prefer-dist --stability dev --no-interaction"

Set this to `true` and `drupal_build_makefile`, `drupal_build_composer` to `false` if you are using Composer's `create-project` as a site deployment strategy.

### Required Drupal site settings

    drupal_core_path: "{{ drupal_deploy_dir }}/web"
    drupal_core_owner: "{{ ansible_ssh_user | default(ansible_env.SUDO_USER, true) | default(ansible_env.USER, true) | default(ansible_user_id) }}"
    drupal_core_owner_become: false

The path to Drupal's root, along with the ownership properties. If you are not running Ansible as the user that should have ownership over the core path, specify the desired system user in `drupal_core_owner` and set `drupal_core_owner_become: true`.

    drupal_db_user: drupal
    drupal_db_password: drupal
    drupal_db_name: drupal
    drupal_db_backend: mysql
    drupal_db_host: "127.0.0.1"

Required Drupal settings. When used in a production or shared environment, you should update at least the `drupal_db_password` and use a secure password.

### Drupal site installation options

    drupal_install_site: true

Set this to `false` if you don't need to install Drupal (using the `drupal_*` settings below), but instead copy down a database (e.g. using `drush sql-sync`).

    drupal_domain: "drupaltest.test"
    drupal_site_name: "Drupal"
    drupal_install_profile: standard
    drupal_site_install_extra_args: []
    drupal_enable_modules: []
    drupal_account_name: admin
    drupal_account_pass: admin

Settings for installing a Drupal site if `drupal_install_site` is `true`. If you need to pass additional arguments to the `drush site-install` command, you can pass them in as a list to the `drupal_site_install_extra_args` variable.

## Dependencies

N/A

## Example Playbook

See the example playbooks used for Travis CI tests (`tests/test.yml` and `tests/test-deploy.yml`) for simple examples. See also: [Drupal VM](https://www.drupalvm.com), which uses this role to set up Drupal.

    - hosts: webserver
      vars_files:
        - vars/main.yml
      roles:
        - geerlingguy.apache
        - geerlingguy.mysql
        - geerlingguy.php-versions
        - geerlingguy.php
        - geerlingguy.php-mysql
        - geerlingguy.composer
        - geerlingguy.drush
        - geerlingguy.drupal

*Inside `vars/main.yml`*:

    drupal_install_site: true
    drupal_build_composer_project: true
    drupal_composer_install_dir: "/var/www/drupal"
    drupal_core_path: "{{ drupal_composer_install_dir }}/web"
    drupal_domain: "example.com"

## License

MIT / BSD

## Author Information

This role was created in 2014 by [Jeff Geerling](https://www.jeffgeerling.com/), author of [Ansible for DevOps](https://www.ansiblefordevops.com/).
