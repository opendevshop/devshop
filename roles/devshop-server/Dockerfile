FROM devshop/ansible:ubuntu1804
LABEL maintainer="Jon Pugh"
ENV ANSIBLE_PLAYBOOK /etc/ansible/playbook.yml
ENV ANSIBLE_OPTIONS_BUILD "-vv --tags build-time"
ENV ANSIBLE_OPTIONS_RUN "-vv --tags run-time"

ENV DOCKER_COMMAND_RUN ansible-playbook \
  $ANSIBLE_PLAYBOOK \
  $ANSIBLE_OPTIONS_RUN

ENV DOCKER_COMMAND_BUILD ansible-playbook \
  $ANSIBLE_PLAYBOOK \
  $ANSIBLE_OPTIONS_BUILD

# Run role-playbook.yml with --tags build-time because this is building a docker container.
COPY role-playbook.yml $ANSIBLE_PLAYBOOK
RUN $DOCKER_COMMAND_BUILD

# Set CMD to ansible-playbook with --tags run-time because this ppay
CMD $DOCKER_COMMAND_RUN
