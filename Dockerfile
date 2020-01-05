#
# DevShop Super Dockerfile
#
# This Dockerfile is designed to be built into any kind of container.
#
# Without any build arguments, Docker will build from the standard `geerlingguy/docker-ubuntu1804-ansible`
#  image, using the `docker/playbook.server.yml` Ansible playbook file.
#
# This is how the official devshop/server:latest image is built:
#
#    docker build .
#
# Useful Build Arguments:
#
#     OS_VERSION  (default: ubuntu1804)
#       Use to specify a different Geerlingguy docker image to build from.
#
#       Available options: https://hub.docker.com/search?q=geerlingguy%2Fdocker-&type=image
#
#         ubuntu1904 ubuntu1804 ubuntu1604 ubuntu1404 ubuntu1204
#         debian10 debian9 debian8
#         centos8 centos7 centos6
#         fedora31 fedora30 fedora29 fedora27 fedora24
#         amazonlinux2
#
#     FROM_IMAGE "geerlingguy/docker-${OS_VERSION}-ansible:latest"
#       Use to specify a full FROM image string. Useful for speeding up the
#       build process. Use FROM_IMAGE=devshop/server to use a pre-configured
#       image instead of building from scratch.
#
#     ANSIBLE_PLAYBOOK_COMMAND_OPTIONS
#       Passed directly to the end of the `ansible-playbook` command.
#
#     ANSIBLE_PLAYBOOK
#       The path to the ansible playbook file you want to run in the build.
#       Relative to devshop repo root.
#
#     ANSIBLE_EXTRA_VARS
#       Converted to EXTRA_VARS, which is consumed by the `ansible-playbook` command.
#       Can be JSON or YML.
#
#     ANSIBLE_VERBOSITY
#       The path to the ansible playbook file you want to run in the build. Relative to devshop repo root.
#
#     ANSIBLE_CONFIG
#       The path to an alternate ansible.cfg file. Relative to devshop repo root.
#
#  Examples:
#
#    1. Build a DevShop Server image from the default image: geerlingguy/docker-ubuntu18-ansible
#
#        docker build .
#
#    2. Build image from geerlingguy/docker-centos7-ansible:
#
#        docker build . --build-arg OS_VERSION=centos7
#
#    3. Rebuild image from `devshop/server:latest`, resulting in a faster build.
#
#        docker build . --build-arg FROM_IMAGE=devshop/server:latest
#
#    4. Pass environment variables to the docker container being built.
#
#       FROM_IMAGE=devshop/server:latest \
#       ANSIBLE_EXTRA_VARS="php_version: 7.4" \
#          docker build . --build-arg FROM_IMAGE --build-arg ANSIBLE_EXTRA_VARS
#
#      When you do not specify a value for a `--build-arg` option, it inherits the
#      execution environment of the `docker build` command.
#
#      This is useful in CI systems like Travis, where you can define environment
#      variables in a in a matrix.
#
#   @TODO: When the robo commands are a little more consisten, put the directions back here.

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
RUN devshop-logo "Hi! Beginning to build Dockerfile from $FROM_IMAGE"

RUN cat /etc/os-release 2>/dev/null || cat /etc/centos-release
RUN ansible --version
RUN set

RUN devshop-logo "Preparing Docker Container Environment..."

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

ARG ANSIBLE_PLAYBOOK=docker/playbook.server.yml
ENV ANSIBLE_PLAYBOOK "${DEVSHOP_PATH}/${ANSIBLE_PLAYBOOK:-docker/playbook.server.yml}"

ARG ANSIBLE_PLAYBOOK_COMMAND_OPTIONS=""
ENV ANSIBLE_PLAYBOOK_COMMAND_OPTIONS ${ANSIBLE_PLAYBOOK_COMMAND_OPTIONS:-""}

# Convert build args into ENV vars that are used by ansible-playbook
# Ansible playbook command line options.
# See https://docs.ansible.com/ansible/latest/cli/ansible-playbook.html

ARG ANSIBLE_CONFIG="${DEVSHOP_PATH}/ansible.cfg"
ENV ANSIBLE_CONFIG ${ANSIBLE_CONFIG:-"${DEVSHOP_PATH}/ansible.cfg"}

ARG ANSIBLE_VERBOSITY=0
ENV ANSIBLE_VERBOSITY ${ANSIBLE_VERBOSITY:-0}

# @TODO These env vars do not seem to work for ansible-playbook.
# The `ansible-playbook --help` output implies that they do, but the docs do not
# show a default value: https://docs.ansible.com/ansible/latest/cli/ansible-playbook.html#cmdoption-ansible-playbook-tags
ARG ANSIBLE_TAGS="all"
ENV TAGS ${ANSIBLE_TAGS:-"all"}

ARG ANSIBLE_SKIP_TAGS="install-devmaster"
ENV SKIP_TAGS ${ANSIBLE_SKIP_TAGS:-"install-devmaster"}

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
--extra-vars="$EXTRA_VARS" \
--tags="$TAGS" \
--skip-tags="$SKIP_TAGS" \
$ANSIBLE_PLAYBOOK_COMMAND_OPTIONS \
"

RUN chmod 766 $DEVSHOP_TESTS_ASSETS_PATH

EXPOSE 80 443 3306 8025
WORKDIR /var/aegir
ENTRYPOINT ["docker-entrypoint"]

# Provision with Ansible!
RUN devshop-logo "Ansible Playbook Environment" && \
  env && \
  devshop-logo "Running Ansible Playbook Command" && \
  echo "" && echo "$ANSIBLE_BUILD_COMMAND" && echo ""

RUN $ANSIBLE_BUILD_COMMAND

RUN devshop-logo "Ansible Playbook Docker Build Complete!" && \
echo "Playbook: $ANSIBLE_PLAYBOOK" && \
echo "Tags: $TAGS" && \
echo "Skip Tags: $SKIP_TAGS" && \
echo "Extra Vars: $EXTRA_VARS" && \
echo "" && \
echo "Ansible Playbook Command:" && \
echo "$ANSIBLE_BUILD_COMMAND" && \
echo "" && \
env | grep "DEVSHOP" && \
env | grep "ANSIBLE"
