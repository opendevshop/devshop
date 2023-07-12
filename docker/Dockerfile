#
# DevShop Role Container Dockerfile Template
#
FROM drupal:php8.1-apache
LABEL maintainer="Jon Pugh"

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       apt-utils \
       build-essential \
       locales \
       software-properties-common \
       rsyslog systemd systemd-cron sudo iproute2 \
    && apt-get clean \
    && rm -Rf /var/lib/apt/lists/* \
    && rm -Rf /usr/share/doc && rm -Rf /usr/share/man

# Fix potential UTF-8 errors with ansible-test.
RUN locale-gen en_US.UTF-8

RUN sed -i 's/^\($ModLoad imklog\)/#\1/' /etc/rsyslog.conf

COPY ./docker/initctl_faker .
RUN chmod +x initctl_faker && rm -fr /sbin/initctl && ln -s /initctl_faker /sbin/initctl

# Remove unnecessary getty and udev targets that result in high CPU usage when using
# multiple containers with Molecule (https://github.com/ansible/molecule/issues/1104)
RUN rm -f /lib/systemd/system/systemd*udev* \
  && rm -f /lib/systemd/system/getty.target

CMD ["/lib/systemd/systemd"]

COPY ./ /usr/share/devshop

# Install Ansible inventory file.
RUN mkdir -p /etc/ansible
RUN echo "[devshop_server]\nlocalhost ansible_connection=local" > /etc/ansible/hosts

ENV DEVSHOP_PATH /usr/share/devshop

# The command to run on container build
ENV DEFAULT_DEVSHOP_DOCKER_COMMAND_BUILD $DEVSHOP_PATH/scripts/devshop-ansible-playbook --tags=buildtime \
  $DEVSHOP_ANSIBLE_BUILDTIME_OPTIONS

# Set DEVSHOP_DOCKER_COMMAND_BUILD from the Default so we can unset it later.
ENV DEVSHOP_DOCKER_COMMAND_BUILD $DEFAULT_DEVSHOP_DOCKER_COMMAND_BUILD

# The command to run when the container starts.
ENV DEFAULT_DEVSHOP_DOCKER_COMMAND_RUN $DEVSHOP_PATH/scripts/devshop-ansible-playbook \
  $DEVSHOP_ANSIBLE_RUNTIME_OPTIONS

# Set DEVSHOP_DOCKER_COMMAND_RUN from the Default so we can unset it later.
ENV DEVSHOP_DOCKER_COMMAND_RUN $DEFAULT_DEVSHOP_DOCKER_COMMAND_RUN

WORKDIR $DEVSHOP_PATH

RUN /usr/share/devshop/install/devshop-install-prerequisites.sh
RUN $DEVSHOP_DOCKER_COMMAND_BUILD
