FROM geerlingguy/docker-ubuntu1804-ansible:latest
LABEL maintainer="Jon Pugh"

ENV DEBIAN_FRONTEND=noninteractive
ENV ANSIBLE_PIPELINING=1
ENV ANSIBLE_CONFIG="/usr/share/devshop/ansible.cfg"
ENV PATH="/usr/share/devshop/bin:$PATH"

ARG ANSIBLE_VERBOSITY=0
ENV ANSIBLE_VERBOSITY ${ANSIBLE_VERBOSITY:-0}

ARG DEVSHOP_USER_UID=1000
ENV DEVSHOP_USER_UID ${DEVSHOP_USER_UID:-1000}

ARG DEVSHOP_PLAYBOOK=docker/playbook.server.yml
ENV DEVSHOP_PLAYBOOK ${DEVSHOP_PLAYBOOK:-docker/playbook.server.yml}

ENV DEVSHOP_PLAYBOOK_PATH="/usr/share/devshop/$DEVSHOP_PLAYBOOK"

ENV DEVSHOP_ENTRYPOINT_LOG_FILES="/var/log/aegir/*"

ENV pip_packages "ansible"

# Use Python3
RUN apt-get update \
    && apt-get remove -y \
       python-setuptools \
       python-pip \
    && apt-get install -y --no-install-recommends \
       python3-setuptools \
       python3-pip

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10

# (re) Install Ansible via Pip(3).
RUN pip install $pip_packages

RUN ansible --version

# Copy DevShop Core to /usr/share/devshop
COPY ./ /usr/share/devshop

# Copy docker shell scripts to /usr/local/bin
COPY ./docker/bin/* /usr/local/bin/

RUN ls -la /usr/local/bin
RUN echo $PATH

# Provision DevShop inside Docker.
RUN ansible-galaxy install -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles
RUN ansible-playbook $DEVSHOP_PLAYBOOK_PATH -e aegir_user_uid=$DEVSHOP_USER_UID -e aegir_user_gid=$DEVSHOP_USER_UID --skip-tags install-devmaster

EXPOSE 80 443 3306 8025
WORKDIR /var/aegir
ENTRYPOINT ["/usr/local/bin/entrypoint"]
CMD ["/lib/systemd/systemd"]
