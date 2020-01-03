# DevShop Base Dockerfile
# This Dockerfile contains global, OS-Agnostic settings.
# The build argument can be used to build DevShop on a different OS.
#
# Without any build arguments, it will build from geerlingguy/docker-ubuntu1804-ansible
#
#  Build Arguments:
#     OS_VERSION.   Use to specify a geerlingguy/docker-*-ansible image.
#     FROM_IMAGE.   Use to specify a full FROM image string.
#
#     @TODO: Document the rest of the build args.
#
#  Examples:
#
#    1. Build image from geerlingguy/docker-centos7-ansible:
#
#      bin/robo up --os-version=centos7
#        - or -
#      docker build . --build-arg OS_VERSION=centos7
#
#    2. Build image from devshop/server:centos
#
#      bin/robo up --from=devshop/server:centos
#        - or -
#      docker build . --build-arg FROM_IMAGE=devshop/server:centos
#
#
ARG OS_VERSION="ubuntu1804"
ARG FROM_IMAGE=geerlingguy/docker-${OS_VERSION}-ansible:latest

FROM $FROM_IMAGE
LABEL maintainer="Jon Pugh"

# Break Cache
ENV OS_VERSION ${OS_VERSION:-"ubuntu1804"}

# Prints out the OS version.
RUN cat /etc/os-release 2>/dev/null || cat /etc/centos-release

ENV ANSIBLE_PIPELINING=1
ENV ANSIBLE_CONFIG="/usr/share/devshop/ansible.cfg"
ENV PATH="/usr/share/devshop/bin:$PATH"

ARG ANSIBLE_VERBOSITY=0
ENV ANSIBLE_VERBOSITY ${ANSIBLE_VERBOSITY:-0}

ARG ANSIBLE_SKIP_TAGS=install-devmaster
ENV ANSIBLE_SKIP_TAGS ${ANSIBLE_SKIP_TAGS:-install-devmaster}

ARG DEVSHOP_USER_UID=1000
ENV DEVSHOP_USER_UID ${DEVSHOP_USER_UID:-1000}

ARG DEVSHOP_PLAYBOOK=docker/playbook.server.yml
ENV DEVSHOP_PLAYBOOK ${DEVSHOP_PLAYBOOK:-docker/playbook.server.yml}

ENV DEVSHOP_PLAYBOOK_PATH="/usr/share/devshop/$DEVSHOP_PLAYBOOK"

ENV DEVSHOP_ENTRYPOINT_LOG_FILES="/var/log/aegir/*"
ENV DEVSHOP_TESTS_ASSETS_PATH="/usr/share/devshop/.github/test-assets"

ENV ANSIBLE_BUILD_COMMAND="ansible-playbook $DEVSHOP_PLAYBOOK_PATH \
    -e aegir_user_uid=$DEVSHOP_USER_UID \
    -e aegir_user_gid=$DEVSHOP_USER_UID \
    --skip-tags $ANSIBLE_SKIP_TAGS \
"

# Copy latest DevShop Core to /usr/share/devshop
COPY ./ /usr/share/devshop
RUN chmod 766 $DEVSHOP_TESTS_ASSETS_PATH

RUN ansible --version

# Install roles inside Docker.
RUN echo "Running: ansible-galaxy install --ignore-errors -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles ..."
RUN ansible-galaxy install --ignore-errors -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles

# Provision DevShop inside Docker.
RUN echo "Running: $ANSIBLE_BUILD_COMMAND ..."
RUN $ANSIBLE_BUILD_COMMAND

EXPOSE 80 443 3306 8025
WORKDIR /var/aegir
ENTRYPOINT ["docker-entrypoint"]
