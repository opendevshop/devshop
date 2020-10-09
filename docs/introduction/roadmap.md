# Roadmap

## Past

For the history of DevShop, see [CHANGELOG.md](http://github.com)

## 1.x Life cycle

### DevShop 1.0.0

**Expected Release:** Q3 2016

The original vision for DevShop is fully automated hosting and scaling. The [Aegir Cloud](http://drupal.org/project/aegir_cloud) and [Aegir Ansible](http://drupal.org/project/aegir_ansible) modules are the final piece to that puzzle.

When the following features are fully complete we will release 1.0.0:

* [Aegir Cloud](http://drupal.org/project/aegir_cloud): Finish support for Amazon, DigitalOcean, Rackspace, and Linode.
* Aegir Ansible: Finish production ready playbooks for Aegir Web, Database, and Load Balancing servers, including the security playbooks and user management.
* [Aegir SSH](http://drupal.org/project/aegir_ssh): Allow per-server user assignment.

### DevShop 1.1.x

The DevShop 1.1.x cycle will focus on user interface improvements and incremental improvements to Aegir.

Many improvements to the Aegir core system are needed to allow work to start on adding support for things like Docker or Wordpress:

* [Decouple Install Task from Site Creation](https://www.drupal.org/node/2754069)
* [Refactor PROVISION\_SITE\_INSTALLED to be more verbose and actually check if site was installed.](https://www.drupal.org/node/2764245)
* [RFC: Refactor how Aegir installs Drupal.](https://www.drupal.org/node/2770077)

### DevShop 1.2.x

In the 1.2.x cycle, we will complete Aegir Docker integration.

Work in progress is available in the [DevShop Rancher](http://github.com/opendevshop/devshop_rancher) project, but is blocked by the issues listed above.

## 2.x

### Drupal 8 & Symfony

Ideally, Aegir 4 will be Symfony components with a Drupal 8 front-end.

DevShop 2 would be a flavor of that.

My hope is for Aegir 4 and DevShop 2 to be completely server and app agnostic.

