FROM geerlingguy/docker-ubuntu1804-ansible:latest
LABEL maintainer="Jon Pugh"

ENV DEBIAN_FRONTEND=noninteractive
ENV ANSIBLE_PIPELINING=1
ENV ANSIBLE_VERBOSITY=1
ENV ENABLE_TASK_DEBUGGER=1
ENV PATH="/usr/share/devshop/bin:$PATH"

ARG DEVSHOP_USER_UID=1000
ENV DEVSHOP_USER_UID ${DEVSHOP_USER_UID:-1000}

ENV pip_packages "ansible"

# Use Python3
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       python3-setuptools \
       python3-pip \
    && rm -Rf /var/lib/apt/lists/* \
    && rm -Rf /usr/share/doc && rm -Rf /usr/share/man \
    && apt-get clean

# (re) Install Ansible via Pip(3).
RUN pip install $pip_packages

# Copy DevShop Core to /usr/share/devshop
COPY ./ /usr/share/devshop

# Copy provisioning directory, variable overrides, and scripts into container.
COPY ./ansible.cfg /etc/ansible/ansible.cfg

# Copy docker shell scripts to /usr/local/bin
COPY ./docker/bin/* /usr/local/bin

RUN ls -la /usr/local/bin
RUN echo $PATH

# Provision DevShop inside Docker.
RUN ansible-galaxy install -r /usr/share/devshop/requirements.yml -p /usr/share/devshop/roles
RUN ansible-playbook /usr/share/devshop/docker/playbook.server.yml -e aegir_user_uid=$DEVSHOP_USER_UID -e aegir_user_gid=$DEVSHOP_USER_UID --skip-tags install-devmaster

EXPOSE 80 443 3306 8025

ENTRYPOINT ["/usr/local/bin/entrypoint"]
CMD ["/lib/systemd/systemd"]
