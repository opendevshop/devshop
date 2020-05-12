set -e

# BASE
# devshop/base is FROM $OS
docker build . --file=base/Dockerfile.centos7 --tag devshop/base:centos7
docker build . --file=base/Dockerfile.ubuntu1804 --tag devshop/base:ubuntu1804

# ANSIBLE
# devshop/ansible is FROM devshop/base
docker build . --file=ansible/Dockerfile.centos7 --tag devshop/ansible:centos7
docker build . --file=ansible/Dockerfile.ubuntu1804 --tag devshop/ansible:ubuntu1804

# DEVSHOP
# devshop/server is FROM devshop/ansible
# @TODO: Build new devshop/server dockerfiles.
# docker build . --file=server/Dockerfile.centos7 --tag devshop/server:centos7
# docker build . --file=server/Dockerfile.ubuntu1804 --tag devshop/server:ubuntu1804
