# DevShop Docker Images

This folder containes all of the various files needed to build the DevShop 
Docker Images. 

### Base Images: OS & SystemD 

- [devshop/base:ubuntu1804](base)
- [devshop/base:centos7](base)

### Ansible Images: PIP & Ansible

Base image plus Ansible, PIP, and helper scripts. 

Other containers can extend these by just changing `ANSIBLE_` server variables. 

- [devshop/ansible:ubuntu1804](ansible)
- [devshop/ansible:centos7](ansible)

*Coming Shortly*

### DevShop Service Images

Built from `devshop/ansible` and the relevant Ansible roles.

- [devshop/server](server) - Fully functional single container devshop server.
- [devshop/http](http) - Apache web server container.

*Coming Shortly*
