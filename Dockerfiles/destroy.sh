#!/usr/bin/env bash
docker-compose kill
docker-compose rm -fv
docker volume rm dockerfiles_aegir
docker volume rm dockerfiles_mysql
