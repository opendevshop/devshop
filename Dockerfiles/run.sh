#!/usr/bin/env bash

# Use --user and --entrypoint to skip the entry script and just run bash as root.
# docker run --rm --name devshop-server --hostname devshop.server.local.computer -ti --entrypoint bash --user=root devshop/server:local

docker run --rm --name devshop-server --hostname devshop.server.local.computer -t -i -e MYSQL_ROOT_PASSWORD=securepass -v devshop-server:/var/aegir -p 80:80 -d devshop/server:local
docker logs -f devshop-server