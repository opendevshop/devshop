#!/usr/bin/env bash

set -e

service apache2 graceful
service cron restart
service mysql restart