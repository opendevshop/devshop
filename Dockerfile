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
#    1. Build a DevShop Server image from the default image: geerlingguy/docker-ubuntu18-ansible
#
#      docker build .
#
#    2. Build image from geerlingguy/docker-centos7-ansible:
#
#      bin/robo up --os-version=centos7
#        - or -
#      docker build . --build-arg OS_VERSION=centos7
#
#    3. Build image from devshop/server:latest with ansible tags 'install-devmaster'
#       This results in a faster build. Use --tags to only run the needed sections of the playbook.
#
#      bin/robo up --from=devshop/server:latest --tags=install-devmaster
#        - or -
#      docker build . --build-arg FROM_IMAGE=devshop/server:latest --build-arg ANSIBLE_TAGS=install-devmaster
#        - or -
#      FROM_IMAGE=devshop/server:latest ANSIBLE_TAGS=install-devmaster docker build . --build-arg FROM_IMAGE --build-arg ANSIBLE_TAGS
#
#    The last method shown demonstrates how to tell docker build to inherit ARGs from the environment.
#    This is useful in CI systems like Travis, where you want to define FROM_IMAGE in an environment matrix.
#
#    The strings listed in the ARG/ENV pairs are the default values. They are
#    used in the default `devshop/server:latest` container on Docker Hub.
#
#    @TODO: Link to more information in Documentation, once there is some.
#

# Set FROM to $FROM_IMAGE variable. This makes this Dockerfile basically universal. :)
# If OS_VERSION is set without FROM_IMAGE, use the geerlingguy image.
ARG OS_VERSION="ubuntu1804"
ARG FROM_IMAGE="geerlingguy/docker-${OS_VERSION}-ansible:latest"
FROM $FROM_IMAGE
LABEL maintainer="Jon Pugh"

# Set ENVs from ARGs that that need to before FROM.
ENV OS_VERSION ${OS_VERSION:-"ubuntu1804"}
ENV FROM_IMAGE ${FROM_IMAGE:-"geerlingguy/docker-${OS_VERSION}-ansible:latest"}

ARG DEVSHOP_PATH="/usr/share/devshop"
ENV DEVSHOP_PATH ${DEVSHOP_PATH:-"/usr/share/devshop"}

# Copy latest DevShop Core to /usr/share/devshop
COPY ./ $DEVSHOP_PATH

# Set PATH so we can run devshop scripts immediately.
ENV PATH="${DEVSHOP_PATH}/bin:$PATH"

# Announce some helpful stuff into the logs.
RUN devshop-logo "Building Dockerfile from $FROM_IMAGE"

RUN cat /etc/os-release 2>/dev/null || cat /etc/centos-release
RUN ansible --version
RUN ansible-playbook --help
RUN set

#
# Prepare Build Args with default values.
#
# The values listed here are the defaults. They define what goes into the main
# `devshop/server:latest` container.
#
# When creating a NEW build arg, use the example below.
#

# Example ARG/ENV pair. Use the same value for "buildArgDefaultValue".
ARG BUILD_ARG_EXAMPLE="buildArgDefaultValue"
ENV BUILD_ARG_EXAMPLE ${BUILD_ARG_EXAMPLE:-"buildArgDefaultValue"}

ARG ANSIBLE_CONFIG="${DEVSHOP_PATH}/ansible.cfg"
ENV ANSIBLE_CONFIG ${ANSIBLE_CONFIG:-"${DEVSHOP_PATH}/ansible.cfg"}

ARG ANSIBLE_VERBOSITY=0
ENV ANSIBLE_VERBOSITY ${ANSIBLE_VERBOSITY:-0}

ARG ANSIBLE_PLAYBOOK="${DEVSHOP_PATH}/docker/playbook.server.yml"
ENV ANSIBLE_PLAYBOOK ${ANSIBLE_PLAYBOOK:-"${DEVSHOP_PATH}/docker/playbook.server.yml"}

ARG ANSIBLE_PLAYBOOK_COMMAND_OPTIONS=""
ENV ANSIBLE_PLAYBOOK_COMMAND_OPTIONS ${ANSIBLE_PLAYBOOK_COMMAND_OPTIONS:-""}




# Prepare Build Args that require alternate environment variable names, such as Ansible playbook command line options.

ARG ANSIBLE_TAGS=""
ENV TAGS ${ANSIBLE_TAGS:-""}

ARG ANSIBLE_SKIP_TAGS="install-devmaster"
ENV SKIP_TAGS ${ANSIBLE_SKIP_TAGS:-""}

# EXTRA_VARS is consumed by `ansible-playbook`.
# YAML or JSON. See https://docs.ansible.com/ansible/latest/cli/ansible-playbook.html
ARG ANSIBLE_EXTRA_VARS=""
ENV EXTRA_VARS ${ANSIBLE_EXTRA_VARS:-""}

# @TODO: Figure out a better way to set ansible extra vars individually.
ARG DEVSHOP_USER_UID=1000
ENV DEVSHOP_USER_UID ${DEVSHOP_USER_UID:-1000}

ENV DEVSHOP_ENTRYPOINT_LOG_FILES="/var/log/aegir/*"
ENV DEVSHOP_TESTS_ASSETS_PATH="${DEVSHOP_PATH}/.github/test-assets"

ENV ANSIBLE_BUILD_COMMAND="ansible-playbook $ANSIBLE_PLAYBOOK \
    -e aegir_user_uid=$DEVSHOP_USER_UID \
    -e aegir_user_gid=$DEVSHOP_USER_UID \
    $ANSIBLE_PLAYBOOK_COMMAND_OPTIONS \
"

RUN chmod 766 $DEVSHOP_TESTS_ASSETS_PATH

# Install roles inside Docker.
# @TODO: Add dependent roles into Git.
RUN echo "Running: ansible-galaxy install --ignore-errors -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles ..."
RUN ansible-galaxy install --ignore-errors -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles

# Provision with Ansible!
RUN devshop-logo "$ANSIBLE_BUILD_COMMAND "

RUN $ANSIBLE_BUILD_COMMAND

EXPOSE 80 443 3306 8025
WORKDIR /var/aegir
ENTRYPOINT ["docker-entrypoint"]
