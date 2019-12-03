FROM geerlingguy/docker-ubuntu1804-ansible:latest
LABEL maintainer="Jon Pugh"

ENV ANSIBLE_PIPELINING=1
ENV ANSIBLE_VERBOSITY=1
ENV ENABLE_TASK_DEBUGGER=1

# Copy DevShop Core to /usr/share/devshop
COPY ./ /usr/share/devshop

# Copy provisioning directory, variable overrides, and scripts into container.
COPY ./ansible.cfg /etc/ansible/ansible.cfg

# Provision DevShop inside Docker.
RUN ansible-galaxy install -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles
RUN ansible-playbook /usr/share/devshop/docker/playbook.server.yml

EXPOSE 80 443 3306 8025

USER root



