#!/usr/bin/env bash
set -e
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PATH="$DIR:$PATH"

ANSIBLE_VERSION=${ANSIBLE_VERSION:-"2.9"}
pip_packages="ansible==${ANSIBLE_VERSION}"

#
# devshop-install-prerequisites.sh
#
# This script prepares the operating system to run Ansible and Python
#
# It was appropriated from Jeff Geerling's Dockerfiles:
#
#  Ubuntu

#  - https://github.com/geerlingguy/docker-ubuntu2004-ansible/blob/master/Dockerfile
#  - https://github.com/geerlingguy/docker-ubuntu1804-ansible/blob/master/Dockerfile
#  - https://github.com/geerlingguy/docker-ubuntu1604-ansible/blob/master/Dockerfile
#
#  CentOS
#  - https://github.com/geerlingguy/docker-centos7-ansible/blob/master/Dockerfile
#  - https://github.com/geerlingguy/docker-centos8-ansible/blob/master/Dockerfile
#
#  Each OS needs slightly different preparation. This script contains logic to work on any of the listed OS.
#

set -e

command_exists() {
	command -v "$@" > /dev/null 2>&1
}

get_distribution() {
	lsb_dist=""
	# Every system that we officially support has /etc/os-release
	if [ -r /etc/os-release ]; then
		lsb_dist="$(. /etc/os-release && echo "$ID")"
	fi
	# Returning an empty string here should be alright since the
	# case statements don't act unless you provide an actual value
	echo "$lsb_dist"
}

prepare_ubuntu1804() {
  PYTHON_DEFAULT=/usr/bin/python3
  DEBIAN_FRONTEND=noninteractive
  apt-get update \
      && apt-get install -y --no-install-recommends \
         apt-utils \
         locales \
         python3-setuptools \
         python3-pip \
         software-properties-common \
         git \
      && rm -Rf /var/lib/apt/lists/* \
      && rm -Rf /usr/share/doc && rm -Rf /usr/share/man \
      && apt-get clean

  # Set Python3 to be the default (allow users to call "python" and "pip" instead of "python3" "pip3"
  update-alternatives --install /usr/bin/python python /usr/bin/python3 1

  pip3 install $pip_packages
}

prepare_centos7() {
    system_packages_pre="\
        deltarpm \
        epel-release \
        initscripts \
        git \
    "
    system_packages="python-pip"

    yum makecache fast
    yum -y install $system_packages_pre
    yum -y update
    yum -y install $system_packages
    yum clean all

    pip install $pip_packages
}

# perform some very rudimentary platform detection
lsb_dist=$( get_distribution )
lsb_dist="$(echo "$lsb_dist" | tr '[:upper:]' '[:lower:]')"

case "$lsb_dist" in
  ubuntu)
    if command_exists lsb_release; then
      dist_version="$(lsb_release --release | cut -f2)"
      case "$dist_version" in
        "14.04")
          dist_version_name="trusty"
        ;;
        "16.04")
          dist_version_name="xenial"
        ;;
        "18.04")
          dist_version_name="bionic"
        ;;
        "20.04")
          dist_version_name="focal"
        ;;
      esac
    fi
    if [ -z "$dist_version" ] && [ -r /etc/lsb-release ]; then
      dist_version="$(. /etc/lsb-release && echo "$DISTRIB_RELEASE")"
      dist_version_name="$(. /etc/lsb-release && echo "$DISTRIB_CODENAME")"
    fi
  ;;

  debian|raspbian)
    dist_version="$(sed 's/\/.*//' /etc/debian_version | sed 's/\..*//')"
    case "$dist_version" in
      10)
        dist_version_name="buster"
      ;;
      9)
        dist_version_name="stretch"
      ;;
      8)
        dist_version_name="jessie"
      ;;
    esac
  ;;

  centos)
    if [ -z "$dist_version" ] && [ -r /etc/os-release ]; then
      dist_version="$(. /etc/os-release && echo "$VERSION_ID")"
    fi
  ;;

  rhel|ol|sles)
    ee_notice "$lsb_dist"
    exit 1
    ;;

  *)
    if command_exists lsb_release; then
      dist_version="$(lsb_release --release | cut -f2)"
    fi
    if [ -z "$dist_version" ] && [ -r /etc/os-release ]; then
      dist_version="$(. /etc/os-release && echo "$VERSION_ID")"
    fi
  ;;

esac

echo "OS Detected: $lsb_dist $dist_version ($dist_version_name)"

# Break out preparation into separate functions.
case "$lsb_dist $dist_version" in
  "ubuntu 18.04")
    prepare_ubuntu1804
  ;;
  "centos 7")
    prepare_centos7
  ;;
esac
