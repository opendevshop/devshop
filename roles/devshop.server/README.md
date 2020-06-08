# Ansible Role: DevShop

Builds and installs [DevShop](https://getdevshop.com/) on a single server or container instance.

It contains Ansible playbook code and Dockerfiles needed to run DevShop on a server or container.

This role is in development. 

## Requirements
 
 This is a meta role. It's purpose is to include all requirements and offer pre-built playbooks.

 However, to even use this role, there are pre-requisites:

 1. Ansible
 2. Git
 3. Other prerequisite packages.

Instead of suggesting you install these, DevShop has a server bootstrap script, available at https://get.devshop.tech

**NOTE:**  Right now, https://get.devshop.tech is just the stock install.sh script. As this branch progresses, it will become a cleaner script that can be shared between the Docker containers and the install.sh script.

## Role Variables

Available variables are listed below, along with default values (see `defaults/main.yml`):

## Example Playbook

See the [default playbook](./role-playbook.yml) for the best example. 

    - hosts: devshop
      vars:
        server_hostname: "devshop.example.com"
      roles:
        - devshop.server

## License

MIT / BSD

## Author Information

This role was created in 2020 by [Jon Pugh](https://jonpugh.github.io/), author of [OpenDevShop](https://www.getdevshop.com/).

It was created by emulating the work of [Jeff Geerling](https://www.jeffgeerling.com/), author of [Ansible for DevOps](https://www.ansiblefordevops.com/).
