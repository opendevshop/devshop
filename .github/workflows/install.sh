# Build a core container and run the install script in it.
# Run from repository root:
# bash .github/scripts/install-container.sh
set -e
docker-compose --file docker/docker-compose.yml build base
docker run --name install-server-test --hostname devshop.local.computer --env DEVSHOP_DOCKER_COMMAND_RUN="bash /usr/share/devshop/install.sh" --publish 80:80 --privileged --volume ./:/usr/share/devshop --volume /sys/fs/cgroup:/sys/fs/cgroup:ro --rm devshop/base bash /usr/share/devshop/install/install.sh
