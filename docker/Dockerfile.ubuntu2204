#
# DevShop Role Container Dockerfile Template
#
FROM ubuntu:22.04
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

COPY initctl_faker .
RUN chmod +x initctl_faker && rm -fr /sbin/initctl && ln -s /initctl_faker /sbin/initctl

# Remove unnecessary getty and udev targets that result in high CPU usage when using
# multiple containers with Molecule (https://github.com/ansible/molecule/issues/1104)
RUN rm -f /lib/systemd/system/systemd*udev* \
  && rm -f /lib/systemd/system/getty.target

CMD ["/lib/systemd/systemd"]
