# Upgrading DevShop

DevShop comes with a command line tool for upgrading itself.

It was released in version 0.4.0.

## `devshop upgrade`

Run `devshop` from the command line to confirm that the CLI is working.

The `devshop upgrade` command requires to be run as root:

```text
$ sudo devshop upgrade
```

The command will automatically lookup the latest release from GitHub and confirm that this is the version you wish to install. It will show you the information it will use like so:

```text
 ╔═══════════════════════════════════════════════════════════════╗ 
 ║           ____  Welcome to  ____  _                           ║ 
 ║          |  _ \  _____   __/ ___|| |__   ___  _ __            ║ 
 ║          | | | |/ _ \ \ / /\___ \|  _ \ / _ \|  _ \           ║ 
 ║          | |_| |  __/\ V /  ___) | | | | (_) | |_) |          ║ 
 ║          |____/ \___| \_/  |____/|_| |_|\___/| .__/           ║ 
 ║                  Upgrade                     |_|              ║ 
 ╚═══════════════════════════════════════════════════════════════╝ 


Current Version: 0.3.1
Checking for latest releases...
Target Version: (Default: 0.4.0) 

UPGRADE OPTIONS
Current Version:  0.3.1
Current DevMaster Path:  /var/aegir/devmaster-0.3.1-2015-09-10-3
Current DevMaster Site:  devshop.local

Target Version:  0.4.0
Target DevMaster Path:  /var/aegir/devmaster-0.4.0
Target DevMaster Makefile:  https://raw.githubusercontent.com/opendevshop/devshop/0.4.0/build-devmaster.make

STEP 1: Upgrade DevMaster
Run the command: drush hostmaster-migrate devshop.local /var/aegir/devmaster-0.4.0 --makefile=https://raw.githubusercontent.com/opendevshop/devshop/0.4.0/build-devmaster.make --root=/var/aegir/devmaster-0.3.1-2015-09-10-3 -y (y/n)
```

Once the devmaster front-end is upgraded, the script will run the "install.sh" script and ansible playbooks to update your server.

## Upgrading from pre-0.4.0

If you have a devshop server with version 0.4.0 or earlier, you can use the devshop upgrade command once you manually update the devshop CLI:

```text
$ cd /usr/share/devshop
$ git fetch
$ git checkout 0.x
$ composer install
```

If you are using an older version, you might also have to add a symlink for the devshop executable:

```text
sudo ln -s /usr/share/devshop/devshop /usr/local/bin/devshop
```

Once you do that, you can run `devshop upgrade` to run through the upgrade process.

## Note on apt-get upgrade

The `devshop upgrade` command does not run `apt-get upgrade` or `yum upgrade` for you, in case this causes problems

As a part of regular maintenance, you should run `sudo apt-get upgrade` or `sudo yum upgrade` to keep your server up to date.

