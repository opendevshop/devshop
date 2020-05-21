# DevShop Docker Images

## `devshop/ansible`

Base Docker images containing Ansible and PIP, built on top of `devshop/base` images.

These images should effectively be exactly the same as `geerlingguy/docker-ubunu1803-ansible`.

## Tags

- `ubuntu1804` `latest` - Ubuntu 18.04 LTS (Bionic)
- `centos7` - Ubuntu 18.04 LTS (Bionic)
- `role-ubuntu1804` - Base for other roles to build FROM using Ubuntu1804
- `role-centos7` -  Base for other roles to build FROM using Centos7
 
## How to Build

This image is built on Docker Hub automatically any time the upstream OS container is rebuilt, and any time a commit is made or merged to the `master` branch. But if you need to build the image on your own locally, do the following:

  1. [Install Docker](https://docs.docker.com/install/).
  2. `cd` into this directory.
  3. Run `docker build --file=base/Dockerfile.ubuntu1804 --tag devshop/base:ubuntu1804-local .`

## How to Use

  1. [Install Docker](https://docs.docker.com/engine/installation/).
  2. Pull this image from Docker Hub: `docker pull devshop/base:ubuntu1804` (or use the image you built earlier, e.g. `devshop/base:ubuntu1804-local`).
  3. Run a container from the image: `docker run --name=test-container --detach --privileged --volume=/sys/fs/cgroup:/sys/fs/cgroup:ro devshop/base:ubuntu1804` 
  4. Use bash inside the container to access a terminal:
    a. `docker exec --tty test-container env TERM=xterm bash`

## Author

Created in 2020 by [Jon Pugh](https://www.github.com/jonpugh/), author of [OpenDevShop](https://getdevshop.com/).

Credits to [Jeff Geerling](https://www.jeffgeerling.com/), author of [Ansible for DevOps](https://www.ansiblefordevops.com/) for all of the SystemD-in-Containers related work.
