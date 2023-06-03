#
# DevShop Role Container Dockerfile Template
#
ARG DEVSHOP_CONTAINER_FROM=ubuntu:20.04
FROM $DEVSHOP_CONTAINER_FROM
ARG DEVSHOP_CONTAINER_FROM

LABEL maintainer="Jon Pugh"
RUN echo "Building container from Dockerfile, from $DEVSHOP_CONTAINER_FROM ..."

ARG DEBIAN_FRONTEND=noninteractive

# Fix for https://github.com/pypa/pip/issues/10219
ARG LANG="en_US.UTF-8"
ARG LC_ALL="en_US.UTF-8"

# For "run-quiet".
ENV OUTPUT all

# Copy devshop source to DEVSHOP_PATH.
ENV DEVSHOP_PATH /usr/share/devshop
ENV PATH="${DEVSHOP_PATH}/bin:${DEVSHOP_PATH}/scripts:${PATH}"

# Prepare server for services and ansible.
RUN echo "Running docker-systemd-prepare to install ansible and prepare container for services ...\n"
ENV DEVSHOP_ANSIBLE_PATH /etc/ansible

# Tell devshop-ansible-playbook what playbookfile to use.
ENV ANSIBLE_PLAYBOOK $DEVSHOP_PATH/roles/devshop.server/play.yml
ENV ANSIBLE_CONFIG $DEVSHOP_PATH/ansible.cfg

# Options passed to ansible-playbook during docker build.ENV DEVSHOP_ANSIBLE_BUILDTIME_OPTIONS "-vv --skip-tags runtime"
ENV DEVSHOP_ANSIBLE_BUILDTIME_OPTIONS ""

# Options passed to ansible-playbook during docker run or exec.
ENV DEVSHOP_ANSIBLE_RUNTIME_OPTIONS ""

# The command to run on container build
ENV DEFAULT_DEVSHOP_DOCKER_COMMAND_BUILD $DEVSHOP_PATH/scripts/devshop-ansible-playbook \
  $DEVSHOP_ANSIBLE_BUILDTIME_OPTIONS

# Set DEVSHOP_DOCKER_COMMAND_BUILD from the Default so we can unset it later.
ENV DEVSHOP_DOCKER_COMMAND_BUILD $DEFAULT_DEVSHOP_DOCKER_COMMAND_BUILD

# The command to run when the container starts.
ENV DEFAULT_DEVSHOP_DOCKER_COMMAND_RUN $DEVSHOP_PATH/scripts/devshop-ansible-playbook \
  $DEVSHOP_ANSIBLE_RUNTIME_OPTIONS

# Set DEVSHOP_DOCKER_COMMAND_RUN from the Default so we can unset it later.
ENV DEVSHOP_DOCKER_COMMAND_RUN $DEFAULT_DEVSHOP_DOCKER_COMMAND_RUN

# Run ansible-playbook again with build-time options.
ARG ANSIBLE_TAGS_ARG
ENV ANSIBLE_TAGS ${ANSIBLE_TAGS_ARG:-buildtime}

# Run ansible-playbook again with build-time options.
ARG ANSIBLE_EXTRA_VARS_ARG
ENV ANSIBLE_EXTRA_VARS ${ANSIBLE_EXTRA_VARS_ARG}

# Run ansible-playbook again with build-time options.
ARG ANSIBLE_PLAYBOOK_COMMAND_OPTIONS_ARG
ENV ANSIBLE_PLAYBOOK_COMMAND_OPTIONS ${ANSIBLE_PLAYBOOK_COMMAND_OPTIONS_ARG:-$ANSIBLE_PLAYBOOK_COMMAND_OPTIONS}

ARG DEVSHOP_DOCKER_COMMAND_BUILD_ARG
ENV DEVSHOP_DOCKER_COMMAND_BUILD ${DEVSHOP_DOCKER_COMMAND_BUILD_ARG:-$DEVSHOP_DOCKER_COMMAND_BUILD}

RUN rm -rf $DEVSHOP_PATH
COPY ./ $DEVSHOP_PATH
RUN echo "Copied DevShop Source into container."

ENV OUTPUT err

# Replace ansible inventory with symlink to devshop.server inventory.
RUN mkdir -p ${DEVSHOP_ANSIBLE_PATH}
RUN rm -rf ${DEVSHOP_ANSIBLE_PATH}/hosts && ln -s ${DEVSHOP_PATH}/roles/devshop.server/inventory ${DEVSHOP_ANSIBLE_PATH}/hosts

RUN ${DEVSHOP_PATH}/docker/bin/docker-systemd-prepare
RUN ${DEVSHOP_PATH}/install/devshop-install-prerequisites.sh

RUN echo "Running $DEVSHOP_ANSIBLE_INVENTORY_INFO_COMMAND ...\n" && \
  $DEVSHOP_ANSIBLE_INVENTORY_INFO_COMMAND

RUN \
  echo "Ansible Directory: `ls -la ${DEVSHOP_ANSIBLE_PATH}`" &&\
  echo "Ansible Inventory: `cat ${DEVSHOP_ANSIBLE_PATH}/hosts`" &&\
  echo "Ansible Inventory: `ansible-inventory --yaml --list`" &&\
  echo "Ansible Playbook: `cat ${ANSIBLE_PLAYBOOK}`" &&\
  echo "Environment: `env`"


# Run the docker build command.
RUN echo "Running $DEVSHOP_DOCKER_COMMAND_BUILD ...\n" && \
  $DEVSHOP_DOCKER_COMMAND_BUILD

# Set ANSIBLE_TAGS back to runtime so the new container uses it.
ENV ANSIBLE_TAGS=runtime

# VOLUME ["/sys/fs/cgroup", "/tmp", "/run"]

WORKDIR /var/aegir

ENV INIT_COMMAND /lib/systemd/systemd
ENTRYPOINT [/usr/share/devshop/docker/bin/docker-systemd-entrypoint]

RUN devshop-logo 'DevShop Server build finished.'
