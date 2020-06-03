![DevShop Logo](./assets/logo-new-light-bg.png)

[DevShop][1] is an **Open Source DevOps Framework** for web applications like Drupal, and a 
set of reusable **components**. It is written in PHP, Ansible, & Drupal, allowing extensive customization.

The goal of DevShop is to provide a fully automated development and systems administration 
experience while remaining as flexible as possible. DevShop leverages [Ansible & Ansible Galaxy][2] for server configuration.

The philosophy of DevShop is simplicity, transparency, and compatibility. Devshop 
works within a standard LAMP stack or inside containers. It can coexist with configuration
management tools like Puppet or Chef, or it can manage itself with Ansible.

## DevShop Components

The following components make up the OpenDevShop Framework:

1. [`devshop/git-split`](https://github.com/devshop-packages/git-split). Commands to split the git monorepo into multiple child repos. Uses the same [splitsh-lite]() script that Symfony and Drupal uses.
    - Reads list of repositories from `composer.json` config.
    - Could be used by Drupal core instead of the "drupalorg" scripts currently in use.
3. [`devshop/power-process`](https://github.com/devshop-packages/power-process). Enhanced Symfony Process component.  
    - Improved command-line user experience, metadata reporting (executed time, PID, logs).
    - Pluggable output through monologger: pipe to screen, file, or remote monologger compatible REST API.
    - Base tool for the rest. Any shell execution should be done through PowerProcess.
    - Migrated from `provision-ops/power-process`.
4. `devshop/yaml-commands`: Migrated from `provision-ops/yaml-tests`. 
5. `drupal/devmaster`: Pushed to drupal.org repo. Ensure compatibility with 
        Drupal.org packaging and composer packagist systems. 
5. devshop/composer-common.
6. devshop/github-api-cli
7. opend

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
