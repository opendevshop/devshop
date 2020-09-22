# DevShop Docker Bin Tools

This repo is split from the main DevShop repository. 
See https://github.com/opendevshop/devshop/tree/1.x/docker

## About

DevShop is a DevOps framework, each component designed to work independently.

These are the scripts used in the Docker components.

## Scripts

In the order they are used:

### `docker-systemd-prepare`

Modifies the container OS (at buildtime) to be able to run SystemD (at runtime). This is inspired by/copied from Jeff Geerling's Docker Containers he built for testing his Ansible roles.   

For simplicity, each supported operating system gets its own function inside this script.

### `docker-systemd-entrypoint`

This "entrypoint" script enables you to use SystemD in a Docker container and still run a "Docker Command" on container launch.

This creates a "virtual machine" like environment where multiple services can be installed. This is necessary to fully test Ansible roles.

**NOTE:** Using this entrypoint requires a privileged container and volume mounts to the linux CGroups path. See any of the READMEs from Geerlingguy's Containers: https://github.com/geerlingguy/docker-ubuntu1804-ansible#how-to-use

**TODO:** Currently if you forget the privilege flag or volume mount, the container exits with obscure errors from SystemD. We should detect the errors and echo into the logs to inform the user what is needed. See [docker-systemd-entrypoint#47](./docker-systemd-entrypoint#47)


#### How it works

1. When used as the Docker entrypoint, the script is launched with PID 1. 
2. Then, the script launches the specified Docker Command as a NEW process (after waiting a few seconds to allow step 3 to commence first.)
3. Finally, the script uses `exec` to launch SystemD, which inherits the PID #1, making SystemD happy to run as it would in a virtual machine. 
 
### `docker-systemd-initctl-faker`

Just a copy of @geerlingguy's script for Ubuntu: https://github.com/geerlingguy/docker-ubuntu1804-ansible/blob/master/initctl_faker

### `run-quiet`

Runs any command without any output but returns the exit code.

Control output by setting an ENV var called `OUTPUT`. 

Useful for noisy commands like `apt-get` or `yum`.

### `docker-get-devshop`

Just a wrapper for the "recommended install method" for devshop.
