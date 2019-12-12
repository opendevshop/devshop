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
ENV DEVSHOP_TESTS_ASSETS_PATH="/var/aegir/.test-assets"

ENV ANSIBLE_BUILD_COMMAND="ansible-playbook $DEVSHOP_PLAYBOOK_PATH \
    -e aegir_user_uid=$DEVSHOP_USER_UID \
    -e aegir_user_gid=$DEVSHOP_USER_UID"

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

# Copy Ansible.cfg to /etc/ansible
COPY ./ansible.cfg /etc/ansible/ansible.cfg

RUN ansible --version

# Copy DevShop Core to /usr/share/devshop
COPY ./ /usr/share/devshop
RUN chmod 766 $DEVSHOP_TESTS_ASSETS_PATH

# Provision DevShop inside Docker.
RUN echo "Running: ansible-galaxy install --ignore-errors -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles ..."
RUN ansible-galaxy install --ignore-errors -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles

# Provision DevShop inside Docker.
RUN echo "Running: $ANSIBLE_BUILD_COMMAND --skip-tags install-devmaster ..."
RUN $ANSIBLE_BUILD_COMMAND --skip-tags install-devmaster

EXPOSE 80 443 3306 8025
WORKDIR /var/aegir
ENTRYPOINT ["docker-entrypoint"]
CMD ["/lib/systemd/systemd"]
