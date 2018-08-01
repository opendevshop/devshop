FROM ubuntu:16.04

ENV AEGIR_VERSION 7.x-3.151
ENV DEVSHOP_VERSION 1.3.4
ENV PROVISION_VERSION 7.x-3.151
ENV REGISTRY_REBUILD_VERSION 7.x-2.5
ENV AEGIR_MAKEFILE https://raw.githubusercontent.com/opendevshop/devshop/$DEVSHOP_VERSION/build-devmaster.make

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && apt-get install -y -qq\
  apache2 \
  cron \
  pwgen \
  git-core \
  libapache2-mod-php \
  libpcre3-dev \
  mysql-client \
  mysql-server \
  nano \
  openssh-server \
  php7.0-common \
  php7.0-curl \
  php7.0-cli \
  php7.0-dev \
  php7.0-fpm \
  php7.0-gd \
  php7.0-imap \
  php7.0-json \
  php7.0-opcache \
  php7.0-xml \
  php7.0-mbstring \
  php7.0-mysql \
  php-sqlite3 \
  php-apcu \
  postfix \
  rsync \
  sudo \
  unzip \
  wget \
  vim


ENV AEGIR_UID 12345

RUN echo "Creating user aegir with UID $AEGIR_UID and GID $AEGIR_GID"

RUN addgroup --gid $AEGIR_UID aegir
RUN adduser --uid $AEGIR_UID --gid $AEGIR_UID --system --shell /bin/bash --home /var/aegir aegir
RUN adduser aegir www-data
RUN a2enmod rewrite
RUN a2enmod ssl
RUN ln -s /var/aegir/config/apache.conf /etc/apache2/conf-available/aegir.conf
RUN ln -s /etc/apache2/conf-available/aegir.conf /etc/apache2/conf-enabled/aegir.conf

RUN wget https://raw.githubusercontent.com/composer/getcomposer.org/1b137f8bf6db3e79a38a5bc45324414a6b1f9df2/web/installer -O - -q | php -- --quiet
RUN cp composer.phar /usr/local/bin/composer

RUN wget https://github.com/drush-ops/drush/releases/download/8.1.17/drush.phar -O - -q > /usr/local/bin/drush
RUN chmod +x /usr/local/bin/composer
RUN chmod +x /usr/local/bin/drush

# Install fix-permissions and fix-ownership scripts
RUN wget http://cgit.drupalcode.org/hosting_tasks_extra/plain/fix_permissions/scripts/standalone-install-fix-permissions-ownership.sh
RUN bash standalone-install-fix-permissions-ownership.sh

# Copy files
COPY files/sudoers-aegir /etc/sudoers.d/aegir
RUN chmod 0440 /etc/sudoers.d/aegir

COPY files/docker-entrypoint.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

COPY files/services-start.sh /usr/local/bin/services-start
RUN chmod +x /usr/local/bin/services-start

COPY files/set-user-ids.sh /usr/local/bin/set-user-ids
RUN chmod +x /usr/local/bin/set-user-ids

# Prepare Aegir Logs folder.
RUN mkdir /var/log/aegir
RUN chown aegir:aegir /var/log/aegir
RUN echo 'Hello, Aegir.' > /var/log/aegir/system.log

# Don't install provision. Downstream tags will do this with the right version.
## Install Provision for all.
RUN mkdir -p /usr/share/drush/commands
RUN drush dl --destination=/usr/share/drush/commands provision-$PROVISION_VERSION -y
RUN drush dl --destination=/usr/share/drush/commands registry_rebuild-$REGISTRY_REBUILD_VERSION -y

# Setup SSH server.
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd
ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile
EXPOSE 22

USER aegir

RUN mkdir /var/aegir/config
RUN mkdir /var/aegir/.drush
RUN mkdir /var/aegir/projects

# You may change this environment at run time. User UID 1 is created with this email address.
ENV AEGIR_CLIENT_EMAIL aegir@aegir.local.computer
ENV AEGIR_CLIENT_NAME admin
ENV AEGIR_PROFILE devmaster
ENV AEGIR_WORKING_COPY 0

# Must be fixed across versions so we can upgrade containers.
ENV AEGIR_HOSTMASTER_ROOT /var/aegir/devmaster

# Build the devmaster stack
RUN drush make $AEGIR_MAKEFILE $AEGIR_HOSTMASTER_ROOT

WORKDIR /var/aegir
VOLUME /var/lib/mysql

EXPOSE 80
EXPOSE 443
EXPOSE 3306

# The Hostname of the database server to use
ENV AEGIR_DATABASE_SERVER localhost
ENV HOME /var/aegir

VOLUME /var/aegir/projects

# docker-entrypoint.sh waits for mysql and runs hostmaster install
ENTRYPOINT ["docker-entrypoint.sh"]
CMD ["drush", "@hostmaster", "hosting-queued"]
