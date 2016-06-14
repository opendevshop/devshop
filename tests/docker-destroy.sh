#!/usr/bin/env bash
docker kill devshop_server
docker rm devshop_server
rm -rf source/devmaster-0.x/sites/devshop.docker
