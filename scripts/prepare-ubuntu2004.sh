#!/usr/bin/env bash

PIP_PACKAGES=${PIP_PACKAGES:-ansible}

apt-get -qq update \
  && apt-get -qq install -y --no-install-recommends \
     apt-utils \
     build-essential \
     locales \
     libffi-dev \
     libssl-dev \
     libyaml-dev \
     python3-dev \
     python3-setuptools \
     python3-pip \
     python3-yaml \
     software-properties-common \
     rsyslog systemd systemd-cron sudo iproute2 \
     curl git \
  && apt-get clean \
  && rm -Rf /var/lib/apt/lists/* \
  && rm -Rf /usr/share/doc && rm -Rf /usr/share/man

sed -i 's/^\($ModLoad imklog\)/#\1/' /etc/rsyslog.conf
locale-gen en_US.UTF-8

 # Install the initctl faker script from @geerlingguy
initctl_faker_url="https://raw.githubusercontent.com/geerlingguy/docker-ubuntu2004-ansible/a7d1e71/initctl_faker"
initctl_faker_path="/sbin/initctl"

curl -ksSL $initctl_faker_url -o $initctl_faker_path
chmod +x $initctl_faker_path

rm -f /lib/systemd/system/systemd*udev* \
  && rm -f /lib/systemd/system/getty.target 

# Set Python3 to be the default (allow users to call "python" and "pip" instead of "python3" "pip3"
update-alternatives --install /usr/bin/python python /usr/bin/python3 1

python -m pip install --upgrade pip

pip3 install $PIP_PACKAGES

mkdir -p /etc/ansible/host_vars
mkdir -p /etc/ansible/group_vars
