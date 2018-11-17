FROM devshop/server

USER root
WORKDIR /root
ENV DOCKER_COMPOSE_VERSION 1.19.0

# Add the docker group and add aegir to it.
ARG DOCKER_GID=1000
RUN addgroup --gid $DOCKER_GID docker
RUN adduser aegir docker

# Install the docker client.
RUN wget -q https://get.docker.com -O /root/get-docker.sh
RUN sh /root/get-docker.sh

# Install docker-compose.
RUN  wget -q "https://github.com/docker/compose/releases/download/$DOCKER_COMPOSE_VERSION/docker-compose-$(uname -s)-$(uname -m)" && \
    cp "docker-compose-$(uname -s)-$(uname -m)" /usr/bin/docker-compose && \
    chmod +x /usr/bin/docker-compose

USER aegir
WORKDIR /var/aegir