#!/usr/bin/env bash
docker kill devshop_server
docker rm devshop_server
rm -rf source/devmaster-1.x/sites/devshop.docker
