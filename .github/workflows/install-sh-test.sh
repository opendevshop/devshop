# Build a core container and run the install script in it.
# Run from repository root:
# bash .github/scripts/install-container.sh
echo "Installing $DEVSHOP_VERSION..."
set -ex

# Rebuild a base container to include this PR's systemd scripts.
docker-compose --file docker/docker-compose.yml build base

# Remove existing install server test containers.
docker kill install-server-test || true

# Launch a devshop/base container with this PR's install.sh script inside.
docker run \
  --name install-server-test \
  --detach --privileged --rm \
  --hostname devshop.local.computer \
  --publish 80:80 \
  --volume $PWD/install/install.sh:/tmp/devshop-install.sh \
  --volume /sys/fs/cgroup:/sys/fs/cgroup:ro \
  devshop/base

docker exec \
  --env="ANSIBLE_PLAYBOOK_COMMAND_OPTIONS=--inventory devshop.server" \
  --env="DEVSHOP_VERSION" \
  install-server-test \
  bash /tmp/devshop-install.sh \
