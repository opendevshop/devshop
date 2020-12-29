# Build a core container and run the install script in it.
# Run from repository root:
# bash .github/scripts/install-container.sh
LOAD_DEVSHOP_VERSION=${LOAD_DEVSHOP_VERSION:-1.x}
DEVSHOP_SERVER_HOSTNAME="install-test.devshop.local.computer"

echo "Running test of install.sh script version $LOAD_DEVSHOP_VERSION..."
echo "  To test a different version, specify the LOAD_DEVSHOP_VERSION environment variable: "
echo ""
echo "  LOAD_DEVSHOP_VERSION=example/branch bash .github/workflows/install-sh-test.sh"

# Remove existing install server test containers.
docker kill install-server-test > /dev/null 2>&1
sleep 2

set -ex

# Rebuild a base container to include this PR's systemd scripts.
docker-compose --file docker/docker-compose.yml build base

cd install
cat build/install.sh | grep $LOAD_DEVSHOP_VERSION

# Launch a devshop/base container with this PR's install.sh script inside.
docker run \
  --name install-server-test \
  --detach --privileged --rm \
  --hostname $DEVSHOP_SERVER_HOSTNAME \
  --publish 80:80 \
  --volume $PWD/build/install.sh:/tmp/devshop-install.sh \
  --volume /sys/fs/cgroup:/sys/fs/cgroup:ro \
  devshop/base

docker exec \
  install-server-test \
  bash /tmp/devshop-install.sh --hostname=$DEVSHOP_SERVER_HOSTNAME \
