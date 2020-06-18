#
# DevShop Ansible Dockerfile Template
#
ARG OS=${OS:-ubuntu1804}
FROM devshop/base:${OS:-ubuntu1804}
LABEL maintainer="Jon Pugh"

# Ansible configuration path
ENV DEVSHOP_ANSIBLE_PATH /etc/ansible

# Install Ansible
# The script is put into /tmp because it is only needed once.
COPY ./bin/docker-ansible-prepare /tmp/docker-ansible-prepare
RUN run-quiet bash /tmp/docker-ansible-prepare

# Prepare Ansible folder, config & inventory
RUN mkdir -p ${DEVSHOP_ANSIBLE_PATH}
COPY ./ansible/inventory  ${DEVSHOP_ANSIBLE_PATH}/hosts
COPY ./ansible/ansible.cfg  ${DEVSHOP_ANSIBLE_PATH}/ansible.cfg

RUN \
echo "\n\n\
DevShop/Ansible Container build complete! \n\n\
Which Ansible? `which ansible && ansible --version`\n\n\
Which Python? `which python && python --version`\n\n\
"
