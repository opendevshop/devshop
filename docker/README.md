# DevShop Dockerfiles

## Base Images

The base images are nothing but the upstream OS, SystemD, and a few
 prerequisite packages like git and sudo.

The [docker-systemd-prepare script](../bin/docker-systemd-prepare) is used 
prepare every supported OS for running SystemD inside the container.

The script is based on the commands inside Jeff Geerling's Docker containers.

- devshop/base:ubuntu1804
- devshop/base:centos7

### Init System

In order to test Ansible roles as they would work on full Linux machines, these
containers run SystemD.

SystemD must run as PID 1 to work. This means the Docker CMD needs to be systemd, normally.

These containers contain a special script that alter the behavior of the container
to run the CMD *after* launching SystemD.

If the command exits with an error, the container exits with one as well.

This allows us to test complex systems that have multiple services running.

By default, there is no Docker command. The entrypoint launches SystemD and stays
open.

## Ansible Images

Base images plus Ansible. Used to build Ansible Roles into containers.

- devshop/ansible:ubuntu1804
- devshop/ansible:centos7

## DevShop Images

- devshop/server - Fully functional single container devshop server.
- devshop/http - Apache web server container.
