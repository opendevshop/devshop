# DevShop Docker Images

## `devshop/ansible:role-$OS`

Base Docker images containing Ansible and PIP, but prepared for other Roles to build from.

Other docker images like `devshop/server` will be FROM this one.

This docker image is designed to make it the children images as simple as possible.

## Tags

- `ubuntu1804` `latest` - Ubuntu 18.04 LTS (Bionic)
- `centos7` - Ubuntu 18.04 LTS (Bionic)

## Author

Created in 2020 by [Jon Pugh](https://www.github.com/jonpugh/), author of [OpenDevShop](https://getdevshop.com/).

Credits to [Jeff Geerling](https://www.jeffgeerling.com/), author of [Ansible for DevOps](https://www.ansiblefordevops.com/) for all of the SystemD-in-Containers related work.
