#!/usr/bin/env bash
service apache2 graceful
service cron restart
service mysql restart