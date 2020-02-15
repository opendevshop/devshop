![DevShop Logo](./assets/logo-new-light-bg.png)

[DevShop][1] is an **Open Source DevOps Framework** for web applications like Drupal, and a 
set of reusable **components**. It is written in PHP, Ansible, & Drupal, allowing extensive customization.

The goal of DevShop is to provide a fully automated development and systems administration 
experience while remaining as flexible as possible. DevShop leverages [Ansible & Ansible Galaxy][2] for server configuration.

The philosophy of DevShop is simplicity, transparency, and compatibility. Devshop 
works within a standard LAMP stack or inside containers. It can coexist with configuration
management tools like Puppet or Chef, or it can manage itself with Ansible.

## DevShop 2020 Development

The main branch of DevShop is now `develop`. Many changes are going into this branch
as soon as possible, such as:

1. Composer Tools & Plugins to be merged into Monorepo and redistributed to Packagist:
    1. `devshop/git-tools`: Git Split & Import tools.
    2. `devshop/bash-tools`: DevShop Bash Tools. New scripts in `./bin` that are useful for all machines.
    3. `devshop/power-process`: Migrated from `provision-ops/power-process`.
    4. `devshop/yaml-commands`: Migrated from `provision-ops/yaml-tests`. 
    5. `devshop/ansible-roles-installer`: New plugin to be created to allow Ansible 
        roles to be defined in `composer.json`.
    6. `devshop/ansible-playbook`: New plugin to allow easy running of "ansible-playbook"
        with the expected options.  
    6. `devshop/ansible-inventory`: New plugin to allow generation of ansible inventory.
    6. `devshop/ansible-hosts`: Command to replace /etc/ansible/hosts: Dynamic inventory provider that reaches out to `server.owner` for configuration.
    7. `devshop/server`: Basic class for managing Servers and their metadata.
    8. `devshop/site`: Basic class for managing Sites and their metadata
    7. `drupal/devmaster`: Pushed to drupal.org repo. Ensure compatibility with 
        Drupal.org packaging and composer packagist systems. 
    
    8. Decide what to do with `provision-ops/provision`. Aegir team is showing interest 
        in 4.x branch. DevShop might stay simpler if we focus on Ansible.
2. Fully Embrace Ansible.
    1. All open source software tools and server config has been solved by Ansible 
        Roles, mostly thanks to @geerlingguy. All DevShop has to do is generate 
        the right variables and playbook and run `ansible-playbook`.
    2. Consider importing [`jonpugh/director`][3] project, or pieces of it.
3. Complete the DevShop CLI toolset.    
    1. Finish the `devshop verify:system` command as the core configuration command for all servers.
    2. Finish Server/Site CRUD commands that work via CLI and WEBUI
    3. Add "Config" component for DevShop CLI config so servers can store basic config.
        1. `server.name`: Unique hostname of this server.
        2. `server.owner`: The name of the server that controls this servers inventory. 
            May be same as `server.name` if server is the parent.
        3. These properties will be used to setup the Ansible hosts 
3. Evaluate `devshop/devmaster` project installation method.
    1. Option 1: Create a Composer Project Template. Every new Devmaster is a separate 
        unique composer project. Users could commit their project to git and easily add new
        modules via `composer require`. Updating the site would just be `git pull && composer install`
    2. Option 2:
        Technically, the main devshop repo could be converted to require devmaster  
       via composer.  Then the Drupal front-end would be right in the `/usr/share/devshop/web` 
       folder. Having a separation between the "devshop platform" and hosted sites
       could be good. 

# DevShop 1.x

![DevShop Project Dashboard](https://github.com/opendevshop/documentation/raw/master/images/devshop.png "A screenshot of the DevShop Project Dashboard")

# Resources

* [Documentation](http://docs.opendevshop.com) Please Contribute! [github.com/opendevshop/documentation](https://github.com/opendevshop/documentation) 
* [Chat](http://gitter.im/opendevshop/devshop) on Gitter: [gitter.im/opendevshop/devshop](http://gitter.im/opendevshop/devshop)
* [Issue Queue](http://github.com/opendevshop/devshop/issues) on GitHub: [github.com/opendevshop/devshop/issues](http://github.com/opendevshop/devshop/issues)
* [Development Information](http://docs.opendevshop.com/development.html)  Developer documentation will walk you through contributing to DevShop.
* [OpenDevShop.com](http://www.opendevshop.com): Company website.
* [Follow us on Twitter](http://twitter.com/opendevshop)

# About DevShop

Version | Status | Aegir | Hosts      | Status
--------|--------|-------|------------|----------
1.x     | Stable | 3.x   | D6, D7, D8 | [![Test Status](https://github.com/opendevshop/devshop/workflows/Tests/badge.svg)](https://github.com/opendevshop/devshop/actions)


[![Backers on Open Collective](https://opencollective.com/devshop/backers/badge.svg)](#backers)
 [![Sponsors on Open Collective](https://opencollective.com/devshop/sponsors/badge.svg)](#sponsors) 

DevShop is a "cloud hosting" system for Drupal. DevShop makes it easy to host, develop, test, and update drupal sites.  It provides a front-end built in Drupal ([Devmaster](https://github.com/opendevshop/devmaster)) and a back-end built with Drush, Symfony, and Ansible.

DevShop deploys your sites using git, and allows you to create unlimited environments for each site.  DevShop makes it very easy to deploy any branch or tag to each environment

Code is deployed on push to your git repo automatically.  Data (the database and files) can be deployed between environments.  Run the built-in hooks whenever code or data is deployed, or write your own.

# Built on Aegir

DevShop utilizes the main components of the Aegir Hosting System: [Hosting](http://drupal.org/project/hosting) and [Provision](http://drupal.org/project/provision). It does not use [Hostmaster](http://drupal.org/project/hostmaster); it uses its own installation profile, [Devmaster](http://github.com/opendevshop/devmaster).  It does not use the theme, Eldir.  The default DevShop theme is called [Boots](https://github.com/opendevshop/devmaster/tree/7.x-1.x/themes/boots) and is included in the Devmaster install profile.

DevShop uses many additional contributed modules that Aegir core does not.

# Aegir Cooperative Founding Member

OpenDevShop Inc is a founding member of the Aegir Cooperative.  Lead DevShop developer Jon Pugh is a core Aegir maintainer.  

See [aegir.coop](http://aegir.coop) for more information.

# Tour

See the [Tour](http://docs.opendevshop.com/tour.html) section of the documentation for a quick walk-through of the DevShop interface.

# Support

* Bug reports and feature requests should be reported in the [DevShop Issue Queue](https://www.github.com/opendevshop/devshop/issues).
* Ask for help in the [Chat Room](http://gitter.im/opendevshop/devshop).

## Contributors

This project exists thanks to all the people who contribute. 
<a href="https://github.com/opendevshop/devshop/graphs/contributors"><img src="https://opencollective.com/devshop/contributors.svg?width=890&button=false" /></a>


## Backers

Thank you to all our backers! 🙏 [[Become a backer](https://opencollective.com/devshop#backer)]

<a href="https://opencollective.com/devshop#backers" target="_blank"><img src="https://opencollective.com/devshop/backers.svg?width=890"></a>

## Sponsors

Support this project by becoming a sponsor. Your logo will show up here with a link to your website. [[Become a sponsor](https://opencollective.com/devshop#sponsor)]

<a href="https://opencollective.com/devshop/sponsor/0/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/0/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/1/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/1/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/2/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/2/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/3/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/3/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/4/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/4/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/5/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/5/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/6/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/6/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/7/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/7/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/8/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/8/avatar.svg"></a>
<a href="https://opencollective.com/devshop/sponsor/9/website" target="_blank"><img src="https://opencollective.com/devshop/sponsor/9/avatar.svg"></a>

For the full list of Backers and Sponsors, see [BACKERS.md](BACKERS.md)


# License

DevShop is licensed under [GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt).

This means any forks of this code must be released as open source and also be licensed under the GPL.

# Help Improve Documentation

For full documentation on DevShop, visit [docs.opendevshop.com](http://docs.opendevshop.com) or see our git repository [github.com/opendevshop/documentation](https://github.com/opendevshop/documentation).

Think this can be improved? You can [Fork our Documentation on GitHub](https://github.com/opendevshop/documentation)!

Thanks!


[1]: https://getdevshop.com
[2]: https://galaxy.ansible.com
[3]: https://github.com/jonpugh/director
