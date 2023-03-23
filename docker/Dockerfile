# The New DevShop Dockerfile.
# See https://www.linuxserver.io/
# syntax=docker/dockerfile:1

FROM lsiobase/ubuntu:focal

# set version label
ARG BUILD_DATE
ARG VERSION
ARG DEVSHOP_VERSION
LABEL build_version="Linuxserver.io version:- ${VERSION} Build-date:- ${BUILD_DATE}"
LABEL maintainer="DevShop"

ARG DEBIAN_FRONTEND=noninteractive

# Fix for https://github.com/pypa/pip/issues/10219
ARG LANG="en_US.UTF-8"
ARG LC_ALL="en_US.UTF-8"


ARG ANSIBLE_HOST_GROUP
ENV ANSIBLE_HOST_GROUP ${ANSIBLE_HOST_GROUP:-devshop_server}

ARG DEVSHOP_PLATFORM_REPOSITORY
ARG DEVSHOP_PLATFORM_PATH

# environment settings
ENV DEVSHOP_VERSION ${DEVSHOP_VERSION:-1.x}
ENV DEVSHOP_PLATFORM_REPOSITORY https://github.com/opendevshop/devshop.git
ENV DEVSHOP_PLATFORM_PATH /usr/share/devshop

# anti linuxserver.io pattern:
RUN echo Installing $DEVSHOP_VERSION from $DEVSHOP_PLATFORM_REPOSITORY
COPY ./ $DEVSHOP_PLATFORM_PATH
RUN cd $DEVSHOP_PLATFORM_PATH && ls -la

# linuxserver.io pattern
# Build server in one "run" command.
RUN \
    # When this becomes a "real" linuxcontainer, install without copy.
    # git clone $DEVSHOP_PLATFORM_REPOSITORY $DEVSHOP_PLATFORM_PATH && \
    cd $DEVSHOP_PLATFORM_PATH && \
    sh scripts/prepare-ubuntu2004.sh && \
    echo "[$ANSIBLE_HOST_GROUP]\nlocalhost ansible_connection=local" > /etc/ansible/hosts && \
    cp ansible.cfg /etc/ansible/ansible.cfg && \
    echo "================================" && \
    echo "Script complete: scripts/prepare-ubuntu2004.sh" && \
    echo "================================"

ENV ANSIBLE_PLAYBOOK /usr/share/devshop/roles/devshop.server/play.yml

RUN \
    echo "Starting devshop-ansible-playbook ..." && \
    export ANSIBLE_PLAYBOOK $ANSIBLE_PLAYBOOK && \
    $DEVSHOP_PLATFORM_PATH/scripts/devshop-ansible-playbook --skip-tags runtime

# linuxserver.io pattern:
# Copy "root" folder to "root" of container after product install.
COPY docker/root/ /
    
EXPOSE 80 443