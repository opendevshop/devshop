# DevShop Docker Images

This folder containes all of the various files needed to build the DevShop 
Docker Images. 

## Architecture

The [DevShop Ansible Roles]() are written in Ansible, using Ansible Galaxy roles
whenever possible. See the [Roles](../roles) folder. 

The [DevShop Docker Images]() use the DevShop Ansible Roles to build, so that there
is only one set of server configuration to maintain.

The [DevShop Docker Images]() are designed to support the Ansible Roles, in order to
provide consistency between Docker container environments and native Linux 
environments.

## Docker Image Tags

All of the `devshop/*` containers share the same tags. These tags also serve as 
the list of supported operating systems.

The tags match the "slugs" for each operating system used by [Jeff Geerling](https://github.com/geerlingguy) in his Ansible roles. See his [Repositories on GitHub](https://github.com/geerlingguy?tab=repositories&q=%22Docker+container+for+Ansible+playbook&type=&language=) that are named `docker-*-ansible`.

* `latest`, `ubuntu1804`
* `centos7`

This list will grow as we finish stabilizing the testing system.

## DevShop Docker Images


1. **devshop/base**

    The base operating system with as few changes as possible. Prepares environment variables and SystemD for running in a container.
2. **devshop/ansible**

    Base image plus Python, PIP, and Ansible installed. Identical to the geerlingguy/docker-*-ansible containers.

3. **devshop/core**

    Ansible image plus PHP, Composer, and the DevShop CLI (including Ansible Roles). 

4. **devshop/role**

    Core image plus the tools needed to easily assign containers a role at runtime or build-time. This container is what all other devshop containers are built from.
    
    This container is designed to self-configure based on environment variables (or Docker's "Build Args", if using at buildtime).
    
## DevShop Role Template

Every DevShop server and container will be based on the **devshop/role** configuration, which will install:
- The DevShop CLI
- The core DevShop Ansible Roles
- Extra Ansible Roles, if any.
- An Ansible inventory, either local or remote, which includes:
  - The server identity and role.
  - The server's peers.
    
The *devshop/role* image is a template for all other server images to be built from.

