FROM devshop/ansible:ubuntu1804
LABEL maintainer="Jon Pugh"

# Run role-playbook.yml with --tags build-time because this is building a docker container.
COPY role-playbook.yml /usr/share/ansible/playbook.yml
RUN ansible-playbook --tags=build-time \
  $ANSIBLE_PLAYBOOK \
  $ANSIBLE_OPTIONS

# Set CMD to ansible-playbook with --tags run-time because this ppay
CMD ansible-playbook --tags=run-time \
  $ANSIBLE_PLAYBOOK \
  $ANSIBLE_OPTIONS
