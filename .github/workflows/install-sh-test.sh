# Build a core container and run the install script in it.
# Run from repository root:
# bash .github/scripts/install-container.sh
LOAD_DEVSHOP_VERSION=${LOAD_DEVSHOP_VERSION:-1.x}
DEVSHOP_SERVER_HOSTNAME="install-test.devshop.local.computer"
OS=${OS:-ubuntu2004}

echo "Running test of install.sh script version $LOAD_DEVSHOP_VERSION on OS $OS..."
echo "  To test a different version, specify the LOAD_DEVSHOP_VERSION environment variable: "
echo ""
echo "  LOAD_DEVSHOP_VERSION=example/branch bash .github/workflows/install-sh-test.sh"
echo ""

# Remove existing install server test containers.
docker kill install-server-test > /dev/null 2>&1
sleep 2

set -ex

cd install

make build

cat build/install.sh | grep $LOAD_DEVSHOP_VERSION

# Rebuild a base container to include this PR's systemd scripts.
docker build . --file Dockerfile --tag ubuntu/container

# Launch a devshop base container with this PR's install.sh script inside.
docker run \
  --name install-server-test \
  --detach --privileged --rm \
  --hostname $DEVSHOP_SERVER_HOSTNAME \
  --publish 80:80 \
  --volume $PWD/build/install.sh:/tmp/devshop-install.sh \
  --tty \
  ubuntu/container

docker exec \
  --tty \
  install-server-test \
  bash /tmp/devshop-install.sh --hostname=$DEVSHOP_SERVER_HOSTNAME \
