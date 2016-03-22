# -*- mode: ruby -*-
# vi: set ft=ruby :

# Require YAML
require 'yaml'
  
VAGRANTFILE_API_VERSION = "2"
Vagrant.require_version ">= 1.5"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

# Load Variables  
  settings = YAML.load_file(File.dirname(__FILE__) + "/vars.yml")
  development_mode = settings["vagrant_development"] || File.exist?(File.dirname(__FILE__) + '/.development_mode')

  # Base Box & Config
  config.vm.box = "ubuntu/trusty64"

  # Uncomment to test with other types of boxes.
  # config.vm.box = "hashicorp/precise64"
  # config.vm.box = "bento/centos-6.5"
  # config.vm.box = "bento/centos-7.1"

  config.vm.provider "virtualbox" do |v|
    v.memory = settings['vagrant_virtualbox_memory']
  end

  # Prepare host for development
  if (development_mode && ARGV[0] == 'up')
    system('bash ' + File.dirname(__FILE__) + '/vagrant-prepare-host.sh ' + File.dirname(__FILE__) + ' ' + settings["devshop_version"])
  end

  # DevShop Master
  # Set to be the default machine.
  # Use `vagrant up` to launch.
  config.vm.define "devmaster", primary: true do |devmaster|
    devmaster.vm.hostname = settings["server_hostname"]
    devmaster.vm.network "private_network", ip: settings["vagrant_private_network_ip"]

    # Set SH as our provisioner
    devmaster.vm.provision "shell",
      path: settings["vagrant_install_script"],
      args: settings["vagrant_install_script_args"]

    # Prepare development environment
    if (development_mode)

      devmaster.vm.synced_folder "source/devmaster-" + settings["devshop_version"], "/var/aegir/devmaster-" + settings["devshop_version"],
          mount_options: ["uid=12345,gid=12345"]

      devmaster.vm.synced_folder "source/drush", "/var/aegir/.drush/commands",
          mount_options: ["uid=12345,gid=12345"]

      # config.vm.synced_folder "source/projects", "/var/aegir/projects",
      #    mount_options: ["uid=12345,gid=12345"]

      devmaster.vm.provision "shell",
        path: 'vagrant-prepare-guest.sh'

      # Make sure settings.php is readable by all users
      devmaster.vm.provision "shell",
        inline: "chmod +r /var/aegir/devmaster-" + settings["devshop_version"] + "/sites/" + settings["server_hostname"] + "/settings.php"

      # Enable some development modules
      devmaster.vm.provision "shell",
        inline: "sudo su - aegir -c 'drush @hostmaster en devel admin_devel -y'"

    end
  end

  # DevShop Remote
  # Does not start automatically on vagrant up.
  # Use `vagrant up remote` to launch.
  config.vm.define "remote", autostart: false do |remote|
    remote.vm.hostname = settings["remote_server_hostname"]
    remote.vm.network "private_network", ip: settings["remote_vagrant_private_network_ip"]
    remote.vm.provider "virtualbox" do |v|
      v.memory = 1024
    end
  end
  config.vm.define "remote2", autostart: false do |remote|
    remote.vm.hostname = settings["remote2_server_hostname"]
    remote.vm.network "private_network", ip: settings["remote2_vagrant_private_network_ip"]
    remote.vm.provider "virtualbox" do |v|
      v.memory = 1024
    end
  end
end

if (ARGV[0] == 'destroy')
  settings = YAML.load_file(File.dirname(__FILE__) + "/vars.yml")
  print "DEVSHOP: Vagrant Destroy detected. \n"
  print "DEVSHOP: You must delete the 'source/devmaster-" + settings["devshop_version"] + "/sites/devshop.site' folder before you 'vagrant up' again. \n"
end

class NoSettingsException < Vagrant::Errors::VagrantError
  error_message('Project settings file not found. Create attributes.json file then try again.')
end
