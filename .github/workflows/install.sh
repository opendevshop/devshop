# Build a core container and run the install script in it.
# Run from repository root:
# bash .github/scripts/install-container.sh
set -e
docker-compose --file docker/docker-compose.yml build base
docker run --name install-server-test --detach --hostname devshop.local.computer --entrypoint /usr/share/devshop/docker/bin/docker-systemd-entrypoint --publish 80:80 --privileged --volume $PWD:/usr/share/devshop --volume /sys/fs/cgroup:/sys/fs/cgroup:ro --rm devshop/base

docker exec --env="ANSIBLE_PLAYBOOK_COMMAND_OPTIONS=--inventory /usr/share/devshop/roles/devshop.server/inventory" install-server-test bash /usr/share/devshop/install/install.sh
