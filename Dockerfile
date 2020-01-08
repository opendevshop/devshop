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
#     ANSIBLE_PLAYBOOK
#       The path to the ansible playbook file you want to run in the build.
#       Relative to devshop repo root.
#       Default: docker/playbook.server.yml
#
#     ANSIBLE_PLAYBOOK_COMMAND_OPTIONS
#       Passed directly to the `ansible-playbook` command.
#
#     ANSIBLE_EXTRA_VARS
#       Value is written to a temporary file and used in the command
#       `ansible-playbook --extra-vars=@tmpfile`
#       Can be JSON or YML.
#        Default: <none>
#
#     ANSIBLE_TAGS
#       Passed to the `--tags` ansible-playbook option.
#       Default: all
#
#     ANSIBLE_SKIP_TAGS
#       Passed to the `--skip-tags` ansible-playbook option.
#       Default: <none>
#
#     ANSIBLE_VERBOSITY
#       The level of verbosity for any `ansible` command. The number of "-v"s.
#       Default: 0
#
#     ANSIBLE_CONFIG
#       The path to an alternate ansible.cfg file. Relative to devshop repo root.
#       Default: ansible.cfg (/usr/share/devshop/ansible.cfg)
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
#    3. Build image with PHP 7.4 by setting an Ansible Variable:
#
#        docker build . --build-arg ANSIBLE_EXTRA_VARS="php_version: 7.4"
#
#    4. Rebuild image from `devshop/server:latest`, and set ANSIBLE_TAGS to
#       "none", resulting in a very short build.
#
#        docker build . --build-arg FROM_IMAGE=devshop/server:latest \
#          --build-arg ANSIBLE_TAGS=none
#
#      This is used for rapid testing of devmaster: the 'install-devmaster' tag
#      is used when running the container, acting as the "install" step.
#
#    5. Pass environment variables to docker build args:
#
#       FROM_IMAGE=devshop/server:latest ANSIBLE_EXTRA_VARS="php_version: 7.4" \
#          docker build . \
#            --build-arg FROM_IMAGE \
#           --build-arg ANSIBLE_EXTRA_VARS
#
#      When you do not specify a value for a `--build-arg` value, it inherits the
#      execution environment of the `docker build` command.
#
#      This is useful in CI systems like Travis, where you can define environment
#      variables in a in a matrix.
#
#      DON'T FORGET: Variables are only passed if you specify the --build-arg.
#      @see .travis.yml file.
#
#   @TODO: When the robo commands are a little more consisten, put the directions back here.

# Set ENVs from ARGs that that need to before FROM.

# If --build-arg OS_VERSION is not set, use 'ubuntu1804'
# NOTE: OS_VERSION is ignored if FROM_IMAGE is set as a build arg.
ARG OS_VERSION="ubuntu1804"

# If --build-arg FROM_IMAGE is not set, use '"geerlingguy/docker-${OS_VERSION}-ansible:latest'
ARG FROM_IMAGE="geerlingguy/docker-${OS_VERSION}-ansible:latest"

FROM $FROM_IMAGE
LABEL maintainer="Jon Pugh"

ENV LINE="echo ────────────────────────────────────────────────────────────────────────────────"

# ENVs need to be set AFTER the FROM statement.
ENV OS_VERSION ${OS_VERSION:-"ubuntu1804"}
ENV FROM_IMAGE ${FROM_IMAGE:-"geerlingguy/docker-${OS_VERSION}-ansible:latest"}

ARG DEVSHOP_PATH="/usr/share/devshop"
ENV DEVSHOP_PATH ${DEVSHOP_PATH:-"/usr/share/devshop"}
WORKDIR $DEVSHOP_PATH

# Set PATH so we can run devshop scripts immediately.
ENV PATH="${DEVSHOP_PATH}/bin:$PATH"

RUN \
    $LINE && echo && \
    echo " Welcome to the DevShop Dockerfile:  Build Phase  " && \
    echo && $LINE && \
    echo "# OS Info: "  && \
    (cat /etc/centos-release 2>/dev/null || cat /etc/os-release 2>/dev/null) && \
    echo && $LINE && \
    echo "# Initial Environment Variables: " && \
    env && \
    $LINE

# Check if DevShop is already installed.
# This happens when FROM is set to a devshop container.
RUN \
    if [ -d $DEVSHOP_PATH ]; then   \
        $LINE; \
        echo " Checking $DEVSHOP_PATH: ! Pre-existing DevShop found !"; \
        ls -la $DEVSHOP_PATH; cat $DEVSHOP_PATH/.git/HEAD; git status; git log -1; \
        echo "Deleting $DEVSHOP_PATH and /var/aegir/* ...";  \
        rm -rf $DEVSHOP_PATH /var/aegir/* /var/aegir/.* 2> /dev/null; \
    else      \
        $LINE; \
        echo " Checking $DEVSHOP_PATH: DevShop Not Present. This is a fresh image"; \
    fi; \
    $LINE; \
    echo "Preparing to copy DevShop source code from host..."; \
    $LINE;

# Copy latest DevShop Core to /usr/share/devshop
COPY ./ $DEVSHOP_PATH

# Announce container information before doing anything.
RUN \
  devshop-logo " Contents of ${DEVSHOP_PATH} after copy"; \
  git status; git log -1; $LINE

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

ARG ANSIBLE_PLAYBOOK="/usr/share/devshop/docker/playbook.server.yml"
ENV ANSIBLE_PLAYBOOK ${ANSIBLE_PLAYBOOK:-"/usr/share/devshop/docker/playbook.server.yml"}

ARG ANSIBLE_PLAYBOOK_COMMAND_OPTIONS=""
ENV ANSIBLE_PLAYBOOK_COMMAND_OPTIONS ${ANSIBLE_PLAYBOOK_COMMAND_OPTIONS:-""}

# Convert build args into ENV vars that are used by ansible-playbook
# Ansible playbook command line options.
# See https://docs.ansible.com/ansible/latest/cli/ansible-playbook.html

ARG ANSIBLE_CONFIG="/usr/share/devshop/ansible.cfg"
ENV ANSIBLE_CONFIG ${ANSIBLE_CONFIG:-"/usr/share/devshop/ansible.cfg"}

ARG ANSIBLE_VERBOSITY=0
ENV ANSIBLE_VERBOSITY ${ANSIBLE_VERBOSITY:-0}

# @TODO These env vars do not seem to work for ansible-playbook.
# The `ansible-playbook --help` output implies that they do, but the docs do not
# show a default value: https://docs.ansible.com/ansible/latest/cli/ansible-playbook.html#cmdoption-ansible-playbook-tags
ARG ANSIBLE_TAGS="all"
ENV ANSIBLE_TAGS ${ANSIBLE_TAGS:-"all"}

ARG ANSIBLE_SKIP_TAGS="install-devmaster"
ENV ANSIBLE_SKIP_TAGS ${ANSIBLE_SKIP_TAGS:-"install-devmaster"}

ARG ANSIBLE_EXTRA_VARS="dockerfile_extra_vars_source: 'ARG default Dockerfile:201'"
ENV ANSIBLE_EXTRA_VARS ${ANSIBLE_EXTRA_VARS:-"dockerfile_extra_vars_source: 'ENV default Dockerfile:202"}

# @TODO: Figure out a better way to set ansible extra vars individually.
ARG DEVSHOP_USER_UID=1000
ENV DEVSHOP_USER_UID ${DEVSHOP_USER_UID:-1000}

ENV DEVSHOP_ENTRYPOINT_LOG_FILES="/var/log/aegir/*"
ENV DEVSHOP_TESTS_ASSETS_PATH="${DEVSHOP_PATH}/.github/test-assets"

ENV ANSIBLE_BUILD_COMMAND="ansible-playbook $ANSIBLE_PLAYBOOK \
-e aegir_user_uid=$DEVSHOP_USER_UID \
-e aegir_user_gid=$DEVSHOP_USER_UID \
--tags="$ANSIBLE_TAGS" \
--skip-tags="$ANSIBLE_SKIP_TAGS" \
$ANSIBLE_PLAYBOOK_COMMAND_OPTIONS \
"

RUN \
  echo "Container Environment Preparation Complete"; \
  devshop-line

# Cleanup unwanted systemd files. See bin/docker-systemd-clean and https://github.com/geerlingguy/docker-ubuntu1804-ansible/pull/12
RUN docker-systemd-clean
RUN chmod 766 $DEVSHOP_TESTS_ASSETS_PATH

# Remove devmaster dir if desired so that devshop code is reinstalled.
ARG DEVSHOP_REMOVE_DEVMASTER=0
ENV DEVSHOP_REMOVE_DEVMASTER ${DEVSHOP_REMOVE_DEVMASTER:-0}
RUN if [ $DEVSHOP_REMOVE_DEVMASTER ]; then rm -rf /var/aegir/devmaster-1.x; fi

# Pre-build Information
RUN \
  devshop-logo "Ansible Playbook Environment" && \
    env && \
  [ -z "$ANSIBLE_EXTRA_VARS" ] && \
    devshop-logo "No extra vars found. Use \"--build-arg ANSIBLE_EXTRA_VARS='var=value var2=value2'\" to alter the build." || \
    devshop-logo "Ansible Playbook Extra Vars Found" && \
    echo $ANSIBLE_EXTRA_VARS

# Provision with Ansible!
RUN \
  devshop-logo "Running: $ANSIBLE_BUILD_COMMAND --extra-vars=\"$ANSIBLE_EXTRA_VARS\"" && \
  $ANSIBLE_BUILD_COMMAND --extra-vars="$ANSIBLE_EXTRA_VARS"

RUN devshop-logo "Ansible Playbook Docker Build Complete!" && \
echo "Playbook: $ANSIBLE_PLAYBOOK" && \
echo "Tags: $ANSIBLE_TAGS" && \
echo "Skip Tags: $ANSIBLE_SKIP_TAGS" && \
echo "Extra Vars: $ANSIBLE_EXTRA_VARS" && \
echo "" && \
echo "Ansible Playbook Command:" && \
echo "$ANSIBLE_BUILD_COMMAND" && \
echo ""

RUN \
    devshop-logo "Wrote build information to /etc/os-release" && \
    env | grep "DEVSHOP" >> /etc/os-release && \
    env | grep "ANSIBLE" >> /etc/os-release && \
    cat  /etc/os-release

EXPOSE 80 443 3306 8025
WORKDIR /var/aegir
CMD ["docker-systemd"]
ENTRYPOINT ["docker-entrypoint"]
