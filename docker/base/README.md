# DevShop Docker Images

## Base Images: OS & SystemD 

Base Docker images containing just the operating system, and some [scripts to 
prepare the image for running SystemD](../bin/docker-systemd-prepare).

These are empty images that can run SystemD so that multiple services can be 
installed and run in a single container.

Other images that build from this can be changed to run single process 
containers when needed. See the [Init System](#init-system) section.

## Tags

- `ubuntu1804` `latest` - Ubuntu 18.04 LTS (Bionic)
- `centos7` - Ubuntu 18.04 LTS (Bionic)
 
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

## Init System

The [docker-systemd-prepare script](../bin/docker-systemd-prepare) is used 
to prepare every supported OS for running SystemD inside the container.

In order to test Ansible roles as they would work on full Linux machines, the
containers must run SystemD so we can install and activate multiple services.

These containers contain a special script that alter the behavior of the container
to run the CMD *after* launching SystemD.

If the command exits with an error, the container exits with one as well. Currently, if a command finishes successfully, the container continues to run.

This allows us to test complex systems that have multiple services running.

To use something other than SystemD for PID 1 in the container, set the
`INIT_COMMAND` environment variable. The INIT_COMMAND is launched by the 
`docker-systemd-entrypoint` script as the same process.

## Author

Created in 2020 by [Jon Pugh](https://www.github.com/jonpugh/), author of [OpenDevShop](https://getdevshop.com/).

Credits to [Jeff Geerling](https://www.jeffgeerling.com/), author of [Ansible for DevOps](https://www.ansiblefordevops.com/) for all of the SystemD-in-Containers related work.
