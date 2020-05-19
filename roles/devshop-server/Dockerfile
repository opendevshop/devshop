FROM devshop/ansible:ubuntu1804
LABEL maintainer="Jon Pugh"

ENV ANSIBLE_PATH /etc/ansible
ENV ANSIBLE_ROLES_PATH /etc/ansible/roles
ENV ANSIBLE_PLAYBOOK /etc/ansible/playbook.yml
ENV ANSIBLE_OPTIONS_BUILD "-vv --tags build-time"
ENV ANSIBLE_OPTIONS_RUN "-vv --tags run-time"

ENV DOCKER_COMMAND_RUN ansible-playbook \
  $ANSIBLE_PLAYBOOK \
  $ANSIBLE_OPTIONS_RUN

ENV DOCKER_COMMAND_BUILD ansible-playbook \
  $ANSIBLE_PLAYBOOK \
  $ANSIBLE_OPTIONS_BUILD

# Copy all ansible roles exactly as they exist in git tree.
COPY ./ $ANSIBLE_ROLES_PATH

# Put ansible.cfg into the default location.
COPY ansible.cfg $ANSIBLE_PATH

# Put role-playbook.yml into the default playbook location.
COPY devshop-server/role-playbook.yml $ANSIBLE_PLAYBOOK

# Put a generic localhost ansible inventory in place. Must be generic during build-time. 
# Dynamic inventory based on HOSTNAME or remote inventory script can only be used at runtime.
RUN echo '[devshop-server]\nlocalhost ansible_connection=local' > /etc/ansible/hosts
RUN cat /etc/ansible/hosts

# Run ansible-playbook with build-time tag.
RUN echo "Running $DOCKER_COMMAND_BUILD... " && $DOCKER_COMMAND_BUILD

# Set CMD to ansible-playbook with run-time tag. 
# REMEMBER: This command is not run until a container using this image is started.
CMD $DOCKER_COMMAND_RUN
