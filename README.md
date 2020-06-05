![DevShop Logo](./assets/logo-new-light-bg.png)

[DevShop][1] is an **Open Source DevOps Framework** and a 
set of reusable **components** designed to improve the web development and 
server management process.

The goal of DevShop is to provide a complete **web development pipeline** out of the
box, while empowering users to choose their own server infrastructure and systems 
architecture.
 
DevShop uses [Ansible & Ansible Galaxy][2] for server configuration, [Symfony 
Console][3] for command line tools, and [Drupal][3] for the web interface.

### Philosophy

The philosophy of DevShop is transparency, simplicity, and modularity:
 
  - **Transparency:** *Control systems at the service level.* 
    
    DevShop manages services like 
    Apache and MySQL using Ansible to give total control to users. Docker images
    are built on top of that, using the same configuration. 
    
    This enables
    parity between traditional servers and the Docker containers, and allows 
    people to use DevShop along side tools they are familiar with like Ansible, Chef and Puppet.
  - **Simplicity:** *Avoid unneeded dependencies on build systems.* 
  
    DevShop manages servers and
    site code like a traditional web server, using Git, MySQL, and Apache to 
    maintain the broadest possible compatibility. 
    
    Build-based systems such as 
    Docker add time and complexity to the deployment process, prevent easy 
    access to servers, and force web developers to learn more skills and tools. 
    
  - **Modularity**: *Allow full customization of servers, processes, and interfaces.* 
  
    DevShop leverages Ansible for server 
    configuration and Drupal for the web interface, providing an easy way 
    to customize of all aspects of the system.
    
    DevShop follows the *core principles of Drupal:* community, collaboration, and
    customization. By following these guidelines, we hope to achieve a sustainable, 
    community-driven platform that can fit most users needs out of the box, without 
    restricting the rest from building their dream system.

## DevShop Components

The following components make up the OpenDevShop Framework:

1. Composer Packages
    1. [`devshop/git-split`](https://github.com/devshop-packages/git-split)
        - Commands to split the git monorepo into multiple child repos. 
        - Uses the same [splitsh-lite]() script that Symfony and Drupal uses.
        - Reads list of repositories from `composer.json` config.
        - In theory, could be used by Drupal core instead of the "drupalorg" scripts currently in use.
    2. [`devshop/composer-common`](https://github.com/devshop-packages/composer-common). Useful tools for any Composer project.
    3. [`devshop/power-process`](https://github.com/devshop-packages/power-process). Enhanced Symfony Process component.  
        - Improved command-line user experience, metadata reporting (executed time, PID, logs).
        - Pluggable output through monologger: pipe to screen, file, or remote monologger compatible REST API.
        - Base tool for the rest. Any shell execution should be done through PowerProcess.
        - Migrated from `provision-ops/power-process`.
    4. [devshop/github-api-cli](https://github.com/devshop-packages/github-api-cli) 
        - Simple CLI wrapper for the GitHub API.
        - Base command posts to any resource, passes any option.
        - Additional commands for specific purposes, such as to create and update "deployments".
    4. [`devshop/yaml-tasks`](https://github.com/devshop-packages/yaml-tasks)
        - Keep tests and standard commands in a Yaml file.
        - Run all commands with a single command.
        - Send command results to GitHub as Commit Status, to show pass/fail results in Pull Request pages.
        - Migrated from `provision-ops/yaml-tests`. 
1. Drupal Projects
    1. [`drupal/devmaster`](https://www.drupal.org): Pushed to drupal.org repo. Ensure compatibility with 
        Drupal.org packaging and composer packagist systems. 
3. Ansible Roles
    1. opendevshop.apache
    2. opendevshop.devmaster
    3. opendevshop.users
    4. devshop.server - Meta role used to create the `devshop/server` container.

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
